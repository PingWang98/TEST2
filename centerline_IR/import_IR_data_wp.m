function image_stack=import_IR_data_wp(cropx1,cropx2,cropy1,cropy2)


if exist('pathname', 'var')
    try
        if isdir(pathname)
            cd(pathname);
        end
    end
end
    
[filename,pathname]  = uigetfile({'*.avi'});
aaa=dir(fullfile(pathname,'*.avi'))  
bbb= size(aaa,1);
FileNames = {aaa.name};
FileNames =sort(FileNames);
ks=[];
for wp=1:1%bbb

    fname = [pathname FileNames{wp}];

    vid=VideoReader(fname);
    
    num_frames=(vid.NumberOfFrames)-1;

    for j=1:num_frames
        ks=read(vid,j);
        ks=ks(:,:,1); 
        img_stack{j,1} = ks(cropy1:cropy2,cropx1:cropx2);

    end
    image_stack{wp} = img_stack

end

end



%answer = inputdlg({'Start frame', 'End frame','number of neurons to track'}, '', 1);
%istart = str2double(answer{1});
%iend = str2double(answer{2});
%num_pts=str2double(answer{3});

%proof_reading(img_stack,[],fname,istart,iend,num_pts);




