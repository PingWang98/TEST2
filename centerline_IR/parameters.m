%find ROI , worm_area_est and filter
cd('E:\cen');
chosen_frame= 300;

curvlim = 0.2;
do_dialog = 1;
do_avi=1;
    if exist('pathname', 'var')
        try
            if isdir(pathname)
            cd(pathname);
            end
        end
    end
    [filename,pathname]  = uigetfile({'*.avi'});  
    readerobj = VideoReader(strcat(pathname,filename));
    
    conc = 15;    
    fps = 25;
    pix_per_mm = 200;
    wormthreshold = 0.1;    
    istart = 1;
    iend = 3000;
    decim = 1;
    filsize = 0.3;
    invert_img = 0;
    do_median_bkg = 0;
    spline_p = 0.001;
    initials = 'human';

decim_filter = ones(decim) / (decim^2);
%  data loading

skip = floor((iend-istart+1)/10);

showcalc = 0;
numframes = iend - istart + 1;
% numframes = floor((iend - istart)/skip) + 1;
deinterlace = 1; interlaceframe = 1;
numcurvpts = 100;

clear mov imgsum imgdata lendata curvdata;

cropyes = 1;
j=0;
for i=istart:skip:iend 
    j = j+1;
    if do_avi
        img=read(readerobj,i);
        img=img(:,:,1);
    end

    img=img(:,:,1);

    if i == istart 
        imgsum = single(img);
        [ysize xsize ] = size(img);
%         imgdata = zeros(ysize, xsize, numframes);
        imgmin = ones(size(img))*255.0;
        imgdata = zeros(ysize, xsize, length(istart:skip:iend));
    end
    figure(1); 
    imagesc(img); colormap gray;hold on;
    axis image;    title(num2str(i));
    imgdata(:,:,j) = img;
    imgmin = min(single(img), single(imgmin)); 
    imgsum = imgsum + single(img);
    pause(0.05);
end

if do_median_bkg
    img_bkg = median(imgdata,3);
else
    img_bkg = imgmin;
end

figure(1);clf;
imagesc(imgsum); colormap gray; hold on;
title('sum image');

text(10,20, 'select ROI: upper left then lower right', 'Color', 'white');
[cropx1 cropy1 ] = ginput(1);
cropx1= floor(cropx1);
cropy1  = floor(cropy1);
plot([1 xsize], [cropy1 cropy1], '-r');
plot([cropx1 cropx1], [1 ysize], '-r');
[cropx2 cropy2 ] = ginput(1); 
cropx2 = floor(cropx2);
cropy2 = floor(cropy2);
plot([1 xsize], [cropy2 cropy2], '-r');
plot([cropx2 cropx2], [1 ysize], '-r');
cropyes = 1;




wormthreshold = 0.17; 

while 1
    j=1;
    i = chosen_frame ; %
    if (i > 9999) 
        constr = '%05d';
    else
        constr = '%04d';
    end   
    if do_avi
        img=read(readerobj,i);
        img=single(img(:,:,1));
    end
    %img = imfilter(img, decim_filter, 'same');
    %img = img(1:decim:end,1:decim:end);
                                                                                                                                                                                                                                                                                                                                         

    img = abs(img(:,:,1)- img_bkg);
%     if deinterlace
%         img(3-interlaceframe:2:end) = img(interlaceframe:2:end);
%     end
    if cropyes
%         imgcrop = img(31:300,151:530);  % 
%         imgcrop = img(81:280,351:600);  %
        imgcrop = img(cropy1:cropy2,cropx1:cropx2);  %
    else
        imgcrop = img;
    end
%     imgcrop = imgcrop';
    figure(1); hold off;

    imagesc(imgcrop); hold on;
    axis image;
    title(strcat(strcat(num2str(j), '/'), num2str(numframes)), 'Interpreter', 'None');

    if j==1
        imgcrop1 = imgcrop;
        colormap jet;
        [ysize xsize ] = size(imgcrop);
%         imgdata = zeros(ysize, xsize, numframes);
        text(10,10,'click on head', 'Color', 'white');
  
        lvl = min(min(imgcrop)) + wormthreshold* (-min(min(imgcrop))+max(max(imgcrop)));
        figure(1);clf;
        imagesc(imgcrop> lvl); axis image; hold on;

        title('click two points separated by worm diameter');
        tmp1 = ginput(1); 
        plot(tmp1(1),tmp1(2), 'ow');
        tmp2 = ginput(1);
        plot(tmp2(1),tmp2(2), 'ow');
        pause(.5);
        worm_diam = norm(tmp1-tmp2);
        title(['worm diameter = ' num2str(worm_diam) ' pixels']);
        worm_area_est = 10*worm_diam^2;
        sizethresh = round(worm_area_est / 2);
        if mod(round(filsize*worm_diam),2)==1
            filradius = round(filsize*worm_diam/2);
        else
            filradius = round(filsize*worm_diam/2)+1;
        end
%         fil = ones(filradius) / prod(filsizeabs);
        fil = fspecial('disk', filradius);
    end
    
    colormap gray;
    clf

    fil = fspecial('disk', 2);
    img2 = conv2(single(imgcrop), fil, 'same');
%     img2 = single(imgcrop);
    qwe=1;
    lvl = min(min(img2))+wormthreshold* (-min(min(img2))+max(max(img2)));
    bw =(img2> lvl*qwe);
    bw2 = bwareaopen(bw,sizethresh);
    bw3 = imcomplement(bw2);
    bw4 = bwareaopen(bw3, sizethresh);
    bw5 = imcomplement(bw4);
    se = offsetstrel('ball',2,2);
    bw6 = imclose(uint8(bw5*255),se);
    bw6=logical(bw6-median(bw6));
    %bw6=logical(bw5-median(bw5));
    tt=0;
    worm_area_est=sum(sum(bw))+300;


%      figure(4);contour((img2-min(min(img2)))/max(max(img2)), [0:0.1:0.5]); colorbar;
%     
%     level = graythresh(imgcrop);
%     bw = im2bw(imgcrop,level);

    % 

    

    STATS = regionprops(bwlabel(bw6),'Area', 'Centroid');
    imshow(bw6);
  break  
end  % end main loop


img_bkg=img_bkg(cropy1:cropy2,cropx1:cropx2);   
save  data1.mat cropy1 cropy2  cropx1  cropx2 worm_area_est fil img_bkg sizethresh




