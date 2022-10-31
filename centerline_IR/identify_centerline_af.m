function [nextheadtail,cv2i] = identify_centerline(imgStack,preheadtail,fil,worm_area_est,sizethresh,img_bkg)


[height,width]=size(imgStack);

curvlim = 0.2;
do_dialog = 1;
do_avi=1;
    j=1;
    conc = 15;    
    fps = 25;
    pix_per_mm = 200;
    wormthreshold = 0.1;    
    decim = 1;
    filsize = 0.3;
    invert_img = 0;
    do_median_bkg = 0;
    spline_p = 0.001;
    initials = 'human';
    numframes=1;
decim_filter = ones(decim) / (decim^2);
numcurvpts = 100;

showcalc = 0;

deinterlace = 1; interlaceframe = 1;
% load(fname3);  % load imgmin
cropyes = 1;
%     p=0.1;  % spline fit parameter
% wormthreshold = 0.15;
% filsize = 10;  % filter for raw img before thresholding
% fil = ones(filsize);

nbfil = ones(3,3); nbfil(2,2)=0; nbfil= single(nbfil); % neighbor filter

curvdata = zeros(numframes,numcurvpts);
areadata = zeros(numframes,1);
centroiddata =  zeros(numframes,2);
% lengthdata =  zeros(numframes,1);
    
cv2i_data = zeros(numframes,numcurvpts+2,2);
angle_data = zeros(numframes,numcurvpts+1);
path1_rescaled_data = zeros(numframes,numcurvpts,2);
path2_rescaled_data = zeros(numframes,numcurvpts,2);

headx=preheadtail(1,1);
heady=preheadtail(2,1);
tailx=preheadtail(1,2);
taily=preheadtail(2,2);




if do_avi

        img=single(imgStack);
end
    %img = imfilter(img, decim_filter, 'same');
    %img = img(1:decim:end,1:decim:end);

     img = abs(img(:,:,1)- img_bkg);
     imgcrop = img;

       img2 = conv2(single(imgcrop), fil, 'same');
%     img2 = single(imgcrop);
      qwe=1;
    lvl = min(min(img2))+wormthreshold* (-min(min(img2))+max(max(img2)));
    bw =(img2> lvl*qwe);
    bw2 = bwareaopen(bw,sizethresh);
    bw3 = imcomplement(bw2);
    bw4 = bwareaopen(bw3, sizethresh);
    bw5 = imcomplement(bw4);
    se = offsetstrel('ball',5,5);
    bw6 = imopen(uint8(bw5*255),se);
    bw6=logical(bw6-median(bw6));
    L = bwlabel(bw6);%标记连通区域
    STATS = regionprops(bwlabel(bw6),'Area', 'Centroid');
                Ar = cat(1, STATS.Area);
            ind = find(Ar ==max(Ar));%找到最大连通区域的标号  
            areadata(1) = STATS(ind,:).Area;
            centroiddata(1,:) = STATS(ind,:).Centroid;
             bw6(find(L~=ind))=0;%将其他区域置为0 
    tt=0;

    while tt==0 
        if sum(sum(bw6))>worm_area_est
           qwe=qwe+0.1;
            bw =(img2> lvl*qwe);
            bw2 = bwareaopen(bw,sizethresh);
            bw3 = imcomplement(bw2);
            bw4 = bwareaopen(bw3, sizethresh);
            bw5 = imcomplement(bw4);
            bw6 = imopen(uint8(bw5*255),se);
            bw6=logical(bw6-median(bw6));
        elseif sum(sum(bw6))<(worm_area_est*2/4)
            qwe=qwe-0.01;
        else
            tt=1;
        end
    end

    L = bwlabel(bw6);%标记连通区域
    STATS = regionprops(bwlabel(bw6),'Area', 'Centroid');
        if size(STATS,1) == 0 
            nextheadtail=[];
            nextheadtail(1,1)=headx;
            nextheadtail(2,1)=heady;
            nextheadtail(1,2)=tailx;
            nextheadtail(2,2)=taily;
            cv2i=[];
        else
    
            Ar = cat(1, STATS.Area);
            ind = find(Ar ==max(Ar));%找到最大连通区域的标号  
            areadata(1) = STATS(ind,:).Area;
            centroiddata(1,:) = STATS(ind,:).Centroid;
             bw6(find(L~=ind))=0;%将其他区域置为0 
            B = bwboundaries(bw6, 'noholes'); %  trace boundary clockwise
            figure(1)
            imagesc(bw6); hold on;
            B1 = B{1};

            B1_size = size(B1,1);

            ksep = ceil(B1_size/20);

            B1_plus = circshift(B1,[ksep 0]);
            B1_minus = circshift(B1,[-ksep 0]);

            AA = B1 - B1_plus;  % AA and BB are vectors between a point on boundary and neighbors +- ksep away
            BB = B1 - B1_minus;

            cAA = AA(:,1) + sqrt(-1)*AA(:,2);
            cBB = BB(:,1) + sqrt(-1)*BB(:,2);

            B1_angle = unwrap(angle(cBB ./ cAA));

        %     figure(31);clf;plot(B1_angle); hold on;

            min1 = find(B1_angle == min(B1_angle),1); % find point on boundary w/ minimum angle between AA, BB
            B1_angle2 = circshift(B1_angle, -min1);
            min2a = round(.25*B1_size)-1+find(B1_angle2(round(.25*B1_size):round(0.75*B1_size))==min(B1_angle2(round(.25*B1_size):round(0.75*B1_size))),1);  % find minimum in other half
            min2 = 1+mod(min2a + min1-1, B1_size);

            tmp = circshift(B1, [1-min1 0]);
            end1 = 1+mod(min2 - min1-1, B1_size);
            path1 = tmp(1:end1,:);
            path2 = tmp(end:-1:end1,:);



            if norm(path1(1,:) - [heady headx]) > norm(path1(end,:) - [heady headx]) % if min1 is at tail, reverse both paths
                tmp = path1;
                path1 = path2(end:-1:1,:);
                path2 = tmp(end:-1:1,:);
            end

            heady = path1(1,1);
            headx = path1(1,2);
            taily = path1(end,1);
            tailx = path1(end,2);

            path_length = numcurvpts;

            path1_rescaled = zeros(path_length,2);
            path2_rescaled = zeros(path_length,2);
            path1_rescaled2 = zeros(path_length,2);
            path2_rescaled2 = zeros(path_length,2);

            path1_rescaled(:,1) = [interp1(0:size(path1,1)-1, path1(:,1), (size(path1,1)-1)*[0:path_length-1]/(path_length-1), 'linear')];
            path1_rescaled(:,2) = [interp1(0:size(path1,1)-1, path1(:,2), (size(path1,1)-1)*[0:path_length-1]/(path_length-1), 'linear')];
            path2_rescaled(:,1) = [interp1(0:size(path2,1)-1, path2(:,1), (size(path2,1)-1)*[0:path_length-1]/(path_length-1), 'linear')];
            path2_rescaled(:,2) = [interp1(0:size(path2,1)-1, path2(:,2), (size(path2,1)-1)*[0:path_length-1]/(path_length-1), 'linear')];

        %     path2_rescaled2 = path2_rescaled;    
            for kk=1:path_length
                tmp1 = repmat(path1_rescaled(kk,:), [path_length,1]) - path2_rescaled;
                tmp2 = sqrt(tmp1(:,1).^2 + tmp1(:,2).^2);
        %         find(tmp2==min(tmp2),1)
                path2_rescaled2(kk,:) = path2_rescaled(find(tmp2==min(tmp2),1),:);
            end

        %     path1_rescaled2 = path1_rescaled;
            for kk=1:path_length
                tmp1 = repmat(path2_rescaled(kk,:), [path_length,1]) - path1_rescaled;
                tmp2 = sqrt(tmp1(:,1).^2 + tmp1(:,2).^2);
        %         find(tmp2==min(tmp2),1)
                path1_rescaled2(kk,:) = path1_rescaled(find(tmp2==min(tmp2),1),:);
            end
            weight_fn = ones(path_length,1);
            tmp=round(path_length*0.2);
            weight_fn(1:tmp)=(0:tmp-1)/tmp;
            weight_fn(end-tmp+1:end)=(tmp-1:-1:0)/tmp;
            weight_fn = [weight_fn weight_fn];


            midline = 0.5*(path1_rescaled+path2_rescaled);
            midline2a = 0.5*(path1_rescaled+path2_rescaled2);
            midline2b = 0.5*(path1_rescaled2+path2_rescaled);
            midline_mixed = midline2a .* weight_fn + midline .* (1-weight_fn);

        %     plot(B1(min1,2),B1(min1,1), 'oy'); hold on;
        %     plot(B1(min2,2),B1(min2,1), 'oy'); hold on;
            figure(1);
            plot(path1_rescaled(1,2),path1_rescaled(1,1), 'or'); hold on;
            plot(path2_rescaled(end,2),path2_rescaled(end,1), 'oy'); hold on;
        %  
            plot(path1_rescaled(1:path_length,2),path1_rescaled(1:path_length,1),'-g'); hold on;
            plot(path2_rescaled(1:path_length,2),path2_rescaled(1:path_length,1), '-r'); hold on;
        %     plot(path2_rescaled2(1:path_length,2),path2_rescaled2(1:path_length,1), '-c'); hold on;
        %     plot(path1_rescaled2(1:path_length,2),path1_rescaled2(1:path_length,1), '-c'); hold on;
        %     plot(midline(:,2),midline(:,1), '-b');
        %     plot(midline2a(:,2),midline2a(:,1), '-w');
        %     plot(midline2b(:,2),midline2b(:,1), '-y');
        %     plot(midline(:,2),midline(:,1), '-c');
            plot(midline_mixed(:,2),midline_mixed(:,1), '-k');

            line = midline_mixed;

            xy = circshift(line, [0 1])'; df = diff(xy,1,2); 
            t = cumsum([0, sqrt([1 1]*(df.*df))]); 
            cv = csaps(t,xy,spline_p);

            figure(1);
            fnplt(cv, '-g'); hold on;   
            plot(centroiddata(1:j,1),centroiddata(1:j,2), ':y'); hold on;


            cv2 =  fnval(cv, t)';
            df2 = diff(cv2,1,1); df2p = df2';

            splen = cumsum([0, sqrt([1 1]*(df2p.*df2p))]);
            lendata(j) = splen(end);

            % interpolate to equally spaced length units
            cv2i = interp1(splen+.00001*[0:length(splen)-1],cv2, [0:(splen(end)-1)/(numcurvpts+1):(splen(end)-1)]);

            % store cv2i data 
            cv2i_data(j,:,:) = cv2i;
            path1_rescaled_data(j,:,:) = path1_rescaled;
            path2_rescaled_data(j,:,:) = path2_rescaled;

            df2 = diff(cv2i,1,1);
            atdf2 =  unwrap(atan2(-df2(:,2), df2(:,1)));
            angle_data(j,:) = atdf2';

            curv = unwrap(diff(atdf2,1)); 
            curvdata(j,:) = curv';



        nextheadtail=[];
        nextheadtail(1,1)=headx;
        nextheadtail(2,1)=heady;
        nextheadtail(1,2)=tailx;
        nextheadtail(2,2)=taily;
        end
end