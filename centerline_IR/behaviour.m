clear,clc
load('data1.mat');
Wormname='03-1'; 
cd('E:\cen\')
SamplePath1 =  'E:\cen\';  %path with files we need
fileExt = '*.avi';  %the suffix of file that will be got

%get all figure's path
files = dir(fullfile(SamplePath1,fileExt)); 
len1 = size(files,1);

%find each figure
fileName = {};
for i=1:1%len1;
   fileName{i} = strcat(SamplePath1,files(i).name);
end


image_stack=import_IR_data_wp(cropx1,cropx2,cropy1,cropy2);
 %for i = 1:length(files)
 %    img_stack{i} = import_micromanager_data
 %end
 
 img_stack_w = {};
 for i = 1:1%length(files)     
   img_stack_w = [img_stack_w;image_stack{i}];
 end
 
dlp=402;
clc
whole_brain_imaging(img_stack_w)     

save  centerline.mat cv2i forward  backward beforereversal beforeturn weird ;
  
 beforeturn(1751:3751)=0;