
function varargout = whole_brain_imaging(varargin)
%several inputs:
%1. image stack
%2. neuronal_position
%3. neuronal_idx
%4. ROIposition

% whole_brain_imaging MATLAB code for whole_brain_imaging.fig
%      whole_brain_imaging, by itself, creates a new whole_brain_imaging or raises the existing
%      singleton*.
%
%      H = whole_brain_imaging returns the handle to a new whole_brain_imaging or the handle to
%      the existing singleton*.
%
%      whole_brain_imaging('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in whole_brain_imaging.M with the given input arguments.
%
%      whole_brain_imaging('Property','Value,...) creates a new whole_brain_imaging or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before whole_brain_imaging_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to whole_brain_imaging_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help whole_brain_imaging

% Last Modified by GUIDE v2.5 17-May-2022 04:18:28

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
    'gui_Singleton',  gui_Singleton, ...
    'gui_OpeningFcn', @whole_brain_imaging_OpeningFcn, ...
    'gui_OutputFcn',  @whole_brain_imaging_OutputFcn, ...
    'gui_LayoutFcn',  [] , ...
    'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT

% --- Executes just before whole_brain_imaging is made visible.
function whole_brain_imaging_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to whole_brain_imaging (see VARARGIN)

% Choose default command line output for whole_brain_imaging

handles.img_stack=varargin{1};
handles.volumes=length(handles.img_stack);
%img_stack{1,1} is the first volume
[handles.image_height,handles.image_width]=size(handles.img_stack{1,1});%{1,1}

width=750;
height=700;

set(hObject,'Units','pixels');
set(handles.figure1, 'Position', [400 400 width height+80]);
set(handles.slider1, 'Position', [0 0 width 18]);
shortname = inputdlg('What is the shortname of this img_stack?','',1);
set(handles.figure1, 'Name', shortname{1});
axes(handles.axes1);
handles.low=0;
handles.high=350;
handles.tracking_threshold=70;
handles.contrast_enhancement=0;
%img_stack{1,1}(:,:,1) is the first volume, and its first slice
if handles.contrast_enhancement
    img=imagesc(contrast_enhancement(handles.img_stack{1,1}(:,:)),[handles.low handles.high]);
else
    img=imagesc(handles.img_stack{1,1}(:,:),[handles.low handles.high]);
end
colormap(gray);
set(handles.axes1, ...
    'Visible', 'off', ...
    'Units', 'pixels', ...
    'Position', [0 30 width height]);

handles.axis_width=width;
handles.axis_height=height;

set(handles.text1, 'Units','pixels');
set(handles.text1, 'Position',[0 44+height+2 width 18]);
set(handles.text1, 'HorizontalAlignment','center');
set(handles.text1, 'String',strcat(num2str(1),'/',num2str(handles.volumes),' volumes; '));

handles.hp=impixelinfo;

set(handles.hp,'Position',[0 44+height+2,300,20]);

set(img,'ButtonDownFcn', 'whole_brain_imaging(''ButtonDown_Callback'',gcbo,[],guidata(gcbo))');

%the maxiumum number of neurons to track is 302
handles.points=cell(302,1);
handles.colorset=varycolor(302);
handles.ROI=cell(handles.volumes,1);

handles.red_green_tform=[];

%initialize neuronal position;
handles.cv2i=cell(handles.volumes,1);
handles.calcium_signal=cell(handles.volumes,1);

handles.weird=zeros(1,handles.volumes);
handles.forward=zeros(1,handles.volumes);
handles.backward=zeros(1,handles.volumes);
handles.beforeturn=zeros(1,handles.volumes);
handles.beforereversal=zeros(1,handles.volumes);



switch length(varargin)
    
    case 1
        
        handles.neuronal_position=cell(handles.volumes,2);
        handles.neuronal_idx=cell(handles.volumes,1);
        handles.ROIposition=cell(handles.volumes,1);
        for i=1:handles.volumes
            handles.ROIposition{i,1}(1,1)=1;
            handles.ROIposition{i,1}(1,2)=1;
            %handles.ROIposition{i,1}(1,3)=round(handles.image_width/2);
            handles.ROIposition{i,1}(1,3)=handles.image_width;
            handles.ROIposition{i,1}(1,4)=handles.image_height;
        end
        
    case 2
        
        disp ('Error: where is the neuronal index.\n');
        
    case 3
        
        handles.neuronal_position=varargin{2};
        handles.neuronal_idx=varargin{3};
        N=max(handles.neuronal_idx{1,1});
        handles.colorset=varycolor(N);
        handles.ROIposition=cell(handles.volumes,1);
        for i=1:handles.volumes
            handles.ROIposition{i,1}(1,1)=1;
            handles.ROIposition{i,1}(1,2)=1;
            %handles.ROIposition{i,1}(1,3)=round(handles.image_width/2);
            handles.ROIposition{i,1}(1,3)=handles.image_width;
            handles.ROIposition{i,1}(1,4)=handles.image_height;
        end
        
    case 4
        
        handles.neuronal_position=varargin{2};
        handles.neuronal_idx=varargin{3};
        handles.ROIposition=varargin{4};
        N=max(handles.neuronal_idx{1,1});
        handles.colorset=varycolor(N);
        
    otherwise
        
        disp('Error: exceeds the number of inputs.\n');
        
end

handles.rg_tform=[];

handles.slider1_is_active=1;

min_step=1/(handles.volumes-1);
max_step=5*min_step;
set(handles.slider1, ...
    'Enable','on', ...
    'Min',1, ...
    'Max',handles.volumes, ...
    'Value',1, ...
    'SliderStep', [min_step max_step]);

handles.current_volume=1;
handles.current_slice=1;
handles.reference_volume=1;

handles.Font_sizeMenuItem=10;

% Show the points if there is any.
if length(varargin)>=3
    
    %red channel
    if ~isempty(handles.neuronal_position{handles.current_volume,1})

        N=size((handles.neuronal_position{handles.current_volume,1}),2);
        neuron_idx=handles.neuronal_idx{handles.current_volume,1};
        handles.colorset=varycolor(max(neuron_idx));
        for j=1:N

            x=handles.neuronal_position{handles.current_volume,1}(1,j);
            y=handles.neuronal_position{handles.current_volume,1}(2,j);

            hold on; 

            handles.points{j}=text(x,y,num2str(neuron_idx(j)));
            set(handles.points{j}, 'Color', handles.colorset(neuron_idx(j),:));
            set(handles.points{j}, 'HorizontalAlignment','center');
            set(handles.points{j}, 'ButtonDownFcn', 'whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
            set(handles.points{j}, 'fontsize',handles.Font_sizeMenuItem);
        end

    end
    
    %green channel
    if ~isempty(handles.neuronal_position{handles.current_volume,2})

        for j=1:N

            x=handles.neuronal_position{handles.current_volume,2}(1,j);
            y=handles.neuronal_position{handles.current_volume,2}(2,j);

            hold on;

            handles.points{j}=text(x,y,num2str(neuron_idx(j)));
            set(handles.points{j},'Color',handles.colorset(neuron_idx(j),:));
            set(handles.points{j},'HorizontalAlignment','center');
            set(handles.points{j},'ButtonDownFcn','whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
            set(handles.points{j},'fontsize',handles.Font_sizeMenuItem);
        end

    end
end

%ROI
if ~isempty(handles.ROIposition{handles.current_volume,1})
    rect=handles.ROIposition{handles.current_volume,1};
    handles.ROI{handles.current_volume,1}=rectangle('Curvature', [0 0],'Position',rect,'EdgeColor','y');

elseif ~isempty(handles.ROIposition{max(handles.current_volume-1,1),1})
    rect=handles.ROIposition{max(handles.current_volume-1,1),1};
    handles.ROI{handles.current_volume,1}=rectangle('Curvature', [0 0],'Position',rect,'EdgeColor','y');
    handles.ROIposition{handles.current_volume,1}=rect;
end

handles.output = hObject;

% Update handles structure
guidata(hObject, handles);
% UIWAIT makes whole_brain_imaging wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = whole_brain_imaging_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --------------------------------------------------------------------
function CloseMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to CloseMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
selection = questdlg(['Close ' get(handles.figure1,'Name') '?'],...
    ['Close ' get(handles.figure1,'Name') '...'],...
    'Yes','No','Yes');
if strcmp(selection,'No')
    return;
end

delete(handles.figure1);


% --- Executes on slider movement.
function slider1_Callback(hObject, eventdata, handles)
% hObject    handle to slider1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider

handles.slider1_is_active=1;

handles.current_volume=round(get(hObject,'Value'));
set(handles.text1, 'String',strcat(num2str(handles.current_volume),'/',num2str(handles.volumes),' volumes; '));
axes(handles.axes1);
cla;
if handles.contrast_enhancement
    img=imagesc(contrast_enhancement(handles.img_stack{handles.current_volume,1}(:,:,handles.current_slice)),[handles.low handles.high]);
else
    img=imagesc(handles.img_stack{handles.current_volume,1}(:,:),[handles.low handles.high]);
end
colormap(gray);
set(handles.axes1, 'Visible', 'off');
set(img,'ButtonDownFcn', 'whole_brain_imaging(''ButtonDown_Callback'',gcbo,[],guidata(gcbo))');

%red channel

if ~isempty(handles.neuronal_position{handles.current_volume,1})
    
    N=size((handles.neuronal_position{handles.current_volume,1}),2);
    neuron_idx=handles.neuronal_idx{handles.current_volume,1};
    handles.colorset=varycolor(max(neuron_idx));
    for j=1:N
        
        x=handles.neuronal_position{handles.current_volume,1}(1,j);
        y=handles.neuronal_position{handles.current_volume,1}(2,j);
        
        hold on;
        
        handles.points{j}=text(x,y,num2str(neuron_idx(j)));
        set(handles.points{j},'Color',handles.colorset(neuron_idx(j),:));
        set(handles.points{j},'HorizontalAlignment','center');
        set(handles.points{j},'ButtonDownFcn','whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
        set(handles.points{j},'fontsize',handles.Font_sizeMenuItem);
    end
    
end

%green channel

if ~isempty(handles.neuronal_position{handles.current_volume,2})
    
    for j=1:N
        
        x=handles.neuronal_position{handles.current_volume,2}(1,j);
        y=handles.neuronal_position{handles.current_volume,2}(2,j);
        
        hold on;
        
        handles.points{j}=text(x,y,num2str(neuron_idx(j)));
        set(handles.points{j},'Color', handles.colorset(neuron_idx(j),:));
        set(handles.points{j},'HorizontalAlignment','center');
        set(handles.points{j},'ButtonDownFcn','whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
        set(handles.points{j},'fontsize',handles.Font_sizeMenuItem);
    end
    
end

%ROI
if ~isempty(handles.ROIposition{handles.current_volume,1})
    rect=handles.ROIposition{handles.current_volume,1};
    handles.ROI{handles.current_volume,1}=rectangle('Curvature', [0 0],'Position',rect,'EdgeColor','y');
    
elseif ~isempty(handles.ROIposition{max(handles.current_volume-1,1),1})
    rect=handles.ROIposition{max(handles.current_volume-1,1),1};
    handles.ROI{handles.current_volume,1}=rectangle('Curvature', [0 0],'Position',rect,'EdgeColor','y');
    handles.ROIposition{handles.current_volume,1}=rect;
end

%pixel intensity
handles.hp=impixelinfo;

set(handles.hp,'Position',[0 44+handles.axis_height+2,300,20]);

guidata(hObject,handles);


% --- Executes during object creation, after setting all properties.
function slider1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to slider1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.

if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end


% ----click on the axes to identify neuronal positions and update neuronal
% position in rest of the frames ------------------
function ButtonDown_Callback(hObject, eventdata, handles)

[x,y]=getcurpt(handles.axes1);

if strcmp(get(handles.figure1,'selectionType'),'alt')
    
    centroids=handles.neuronal_position{handles.current_volume,1};
    neuron_idx=handles.neuronal_idx{handles.current_volume,1};
    N=size(centroids,2);
    missing_idx=find_missing_idx(neuron_idx);
    
    prompt = {'Enter Neuron Index:'};
    dlg_title = 'Neuronal Identification';
    num_lines = 1;
    def = {num2str(missing_idx)};
    answer = inputdlg(prompt,dlg_title,num_lines,def);
    
    if ~isempty(answer)
        idx=str2num(answer{1});
        centroids(:,N+1)=[x;y];
        neuron_idx(N+1)=idx;
        
        handles.neuronal_position{handles.current_volume,1}=centroids;
        handles.neuronal_idx{handles.current_volume,1}=neuron_idx;
        handles.colorset=varycolor(max(neuron_idx));
        
        axes(handles.axes1);
        hold on;
        
        handles.points{N+1}=text(x,y,num2str(neuron_idx(N+1)));
        set(handles.points{N+1},'Color',handles.colorset(neuron_idx(N+1),:));
        set(handles.points{N+1},'HorizontalAlignment','center');
        set(handles.points{N+1},'ButtonDownFcn','whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
        set(handles.points{N+1},'fontsize',handles.Font_sizeMenuItem);
        hObject=handles.points{N+1};
        guidata(hObject,handles);
        
    end
    
end

if strcmp(get(handles.figure1,'selectionType'),'normal')
    
    centroids=handles.neuronal_position{handles.current_volume,1};
    neuron_idx=handles.neuronal_idx{handles.current_volume,1};
    N=size(centroids,2);
    missing_idx=find_missing_idx(neuron_idx);
    
    def = {num2str(missing_idx)};
    if def{1} ~= 0
        answer = def;
    else
        answer{1} = num2str(1);
    end
    
    if ~isempty(answer)
        idx=str2num(answer{1});
        centroids(:,N+1)=[x;y];
        neuron_idx(N+1)=idx;
        
        handles.neuronal_position{handles.current_volume,1}=centroids;
        handles.neuronal_idx{handles.current_volume,1}=neuron_idx;
        handles.colorset=varycolor(max(neuron_idx));
        
        axes(handles.axes1);
        hold on;
                
        handles.points{N+1}=text(x,y,num2str(neuron_idx(N+1)));
        set(handles.points{N+1},'Color',handles.colorset(neuron_idx(N+1),:));
        set(handles.points{N+1},'HorizontalAlignment','center');
        set(handles.points{N+1},'ButtonDownFcn','whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
        set(handles.points{N+1},'fontsize',handles.Font_sizeMenuItem);
        hObject=handles.points{N+1};
        guidata(hObject,handles);
        
    end
    
end


% ----click on the axes to identify neuronal positions and update neuronal
% position in rest of the frames ------------------
function ButtonDownPoint_Callback(hObject, eventdata, handles)

[x,y]=getcurpt(handles.axes1);

if strcmp( get(handles.figure1,'selectionType') , 'extend')
    
    centroids=handles.neuronal_position{handles.current_volume,1};
    neuron_idx=handles.neuronal_idx{handles.current_volume,1};
    N=size(centroids,2);
    
    distance_square=sum((centroids(1:2,:)'-repmat([x y],N,1)).^2,2);
    distance=sqrt(distance_square);
    [~,k]=min(distance);
    neuron_idx(k)=[];
    centroids(:,k)=[];
    
    handles.neuronal_position{handles.current_volume,1}=centroids;
    handles.neuronal_idx{handles.current_volume,1}=neuron_idx;
    
    delete(hObject);
    
    handles.colorset=varycolor(max(neuron_idx));
    
    guidata(gcf,handles);
    
end


% --------------------------------------------------------------------
function SaveMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to SaveMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
guidata(hObject,handles);
save  centerline.mat cv2i forward  backward beforereversal beforeturn weird 
guidata(hObject,handles);
% assignin('base','forward',handles.forward);
% assignin('base','beforebackward',handles.backward);
% assignin('base','beforereversal',handles.beforereversal);
% assignin('base','beforeturn',handles.beforeturn);
% assignin('base','weird',handles.weird);
% assignin('base','cv2i',handles.cv2i);
% assignin('base','handles',handles);

% --------------------------------------------------------------------
function ExportMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to ExportMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
assignin('base','forward',handles.forward);
assignin('base','backward',handles.backward);
assignin('base','beforereversal',handles.beforereversal);
assignin('base','beforeturn',handles.beforeturn);
assignin('base','weird',handles.weird);
assignin('base','cv2i',handles.cv2i);
assignin('base','handles',handles);
t = datetime;
assignin('base','t',t);
disp('Data exported at: ');
disp(t);
guidata(hObject,handles);

% --------------------------------------------------------------------
function LUTMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to LUTMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
answer = inputdlg({'Low','High'}, 'Cancel to Clear Previous', 1, ...
    {num2str(handles.low),num2str(handles.high)});
handles.low=str2double(answer{1});
handles.high=str2double(answer{2});
axes(handles.axes1);
cla;
img=imagesc(handles.img_stack{handles.current_volume,1}(:,:),[handles.low handles.high]);
colormap(gray);
set(img,'ButtonDownFcn', 'whole_brain_imaging(''ButtonDown_Callback'',gcbo,[],guidata(gcbo))');

guidata(hObject,handles);


function figure1_WindowScrollWheelFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  structure with the following fields (see FIGURE)
%	VerticalScrollCount: signed integer indicating direction and number of clicks
%	VerticalScrollAmount: number of lines scrolled for each click
% handles    structure with handles and user data (see GUIDATA)

if eventdata.VerticalScrollCount>0
    if handles.slider1_is_active &&(handles.current_volume<handles.volumes)
        handles.current_volume=handles.current_volume+1;
    end
elseif eventdata.VerticalScrollCount<0
    if handles.slider1_is_active && (handles.current_volume>1)
        handles.current_volume=handles.current_volume-1;
    end
end

set(handles.slider1,'Value',handles.current_volume);

set(handles.text1, 'String',strcat(num2str(handles.current_volume),'/',num2str(handles.volumes),' volumes; '));
axes(handles.axes1);
cla;
if handles.contrast_enhancement
    img=imagesc(contrast_enhancement(handles.img_stack{handles.current_volume,1}(:,:)),[handles.low handles.high]);
else
    img=imagesc(handles.img_stack{handles.current_volume,1}(:,:),[handles.low handles.high]);
end
colormap(gray);
set(handles.axes1, 'Visible', 'off');
set(img,'ButtonDownFcn', 'whole_brain_imaging(''ButtonDown_Callback'',gcbo,[],guidata(gcbo))');

%red channel
if ~isempty(handles.neuronal_position{handles.current_volume,1})
    
    N=size((handles.neuronal_position{handles.current_volume,1}),2);
    neuron_idx=handles.neuronal_idx{handles.current_volume,1};
    handles.colorset=varycolor(max(neuron_idx));
    for j=1:N
        
        x=handles.neuronal_position{handles.current_volume,1}(1,j);
        y=handles.neuronal_position{handles.current_volume,1}(2,j);
        
        hold on;  
        
        handles.points{j}=text(x,y,num2str(neuron_idx(j)));
        set(handles.points{j},'Color',handles.colorset(neuron_idx(j),:));
        set(handles.points{j},'HorizontalAlignment','center');
        set(handles.points{j},'ButtonDownFcn','whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
        set(handles.points{j},'fontsize',handles.Font_sizeMenuItem);
    end
    
end

%green channel
if ~isempty(handles.neuronal_position{handles.current_volume,2})
    
    
    for j=1:N
        
        x=handles.neuronal_position{handles.current_volume,2}(1,j);
        y=handles.neuronal_position{handles.current_volume,2}(2,j);
        
        hold on;
        
        handles.points{j}=text(x,y,num2str(neuron_idx(j)));
        set(handles.points{j},'Color',handles.colorset(neuron_idx(j),:));
        set(handles.points{j},'HorizontalAlignment','center');
        set(handles.points{j},'ButtonDownFcn','whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
        set(handles.points{j},'fontsize',handles.Font_sizeMenuItem);
    end
    
end

%ROI
if ~isempty(handles.ROIposition{handles.current_volume,1})
    rect=handles.ROIposition{handles.current_volume,1};
    handles.ROI{handles.current_volume,handles.current_slice}=rectangle('Curvature', [0 0],'Position',rect,'EdgeColor','y');
    
elseif ~isempty(handles.ROIposition{max(handles.current_volume-1,1),1})
    rect=handles.ROIposition{max(handles.current_volume-1,1),1};
    handles.ROI{handles.current_volume,1}=rectangle('Curvature', [0 0],'Position',rect,'EdgeColor','y');
    handles.ROIposition{handles.current_volume,1}=rect;
end

%online pixel intensity calculation
handles.hp=impixelinfo;

set(handles.hp,'Position',[0 44+handles.axis_height+2,300,20]);

guidata(hObject,handles);


% --- Executes on key press with focus on slider1 and none of its controls.
function figure1_KeyPressFcn(hObject, eventdata, handles)
% hObject    handle to slider1 (see GCBO)
% eventdata  structure with the following fields (see MATLAB.UI.CONTROL.UICONTROL)
%	Key: name of the key that was pressed, in lower case
%	Character: character interpretation of the key(s) that was pressed
%	Modifier: name(s) of the modifier key(s) (i.e., control, shift) pressed
% handles    structure with handles and user data (see GUIDATA)
jumping = 5;
if eventdata.Key=='s'
    if handles.slider1_is_active &&(handles.current_volume<handles.volumes-jumping)
        handles.current_volume=handles.current_volume+jumping;
    end
elseif  eventdata.Key=='w'
    if handles.slider1_is_active && (handles.current_volume>jumping)
        handles.current_volume=handles.current_volume-jumping;
    end
end
Bigjumping = 20;
if eventdata.Key=='d'
    if handles.slider1_is_active &&(handles.current_volume<handles.volumes-Bigjumping)
        handles.current_volume=handles.current_volume+Bigjumping;
    end
elseif  eventdata.Key=='a'
    if handles.slider1_is_active && (handles.current_volume>Bigjumping)
        handles.current_volume=handles.current_volume-Bigjumping;
    end
end
Superjumping = 50;
if eventdata.Key=='e'
    if handles.slider1_is_active &&(handles.current_volume<handles.volumes-Superjumping)
        handles.current_volume=handles.current_volume+Superjumping;
    end
elseif  eventdata.Key=='q'
    if handles.slider1_is_active && (handles.current_volume>Superjumping)
        handles.current_volume=handles.current_volume-Superjumping;
    end
end

set(handles.slider1,'Value',handles.current_volume);

set(handles.text1, 'String',strcat(num2str(handles.current_volume),'/',num2str(handles.volumes),' volumes; '));
axes(handles.axes1);
cla;
if handles.contrast_enhancement
    img=imagesc(contrast_enhancement(handles.img_stack{handles.current_volume,1}(:,:)),[handles.low handles.high]);
else
    img=imagesc(handles.img_stack{handles.current_volume,1}(:,:),[handles.low handles.high]);
end
colormap(gray);
set(handles.axes1, 'Visible', 'off');
set(img,'ButtonDownFcn', 'whole_brain_imaging(''ButtonDown_Callback'',gcbo,[],guidata(gcbo))');

%red channel

if ~isempty(handles.neuronal_position{handles.current_volume,1})
    
    N=size((handles.neuronal_position{handles.current_volume,1}),2);
    neuron_idx=handles.neuronal_idx{handles.current_volume,1};
    handles.colorset=varycolor(max(neuron_idx)); %a bug fixed by fanchan
    for j=1:N
        
        x=handles.neuronal_position{handles.current_volume,1}(1,j);
        y=handles.neuronal_position{handles.current_volume,1}(2,j);
        
        hold on; 
        
        handles.points{j}=text(x,y,num2str(neuron_idx(j)));
        set(handles.points{j},'Color',handles.colorset(neuron_idx(j),:));
        set(handles.points{j},'HorizontalAlignment','center');
        set(handles.points{j},'ButtonDownFcn','whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
        set(handles.points{j},'fontsize',handles.Font_sizeMenuItem);
    end
    
end

%green channel

if ~isempty(handles.neuronal_position{handles.current_volume,2})
    
    
    for j=1:N
        
        x=handles.neuronal_position{handles.current_volume,2}(1,j);
        y=handles.neuronal_position{handles.current_volume,2}(2,j);
        
        hold on; 
        
        handles.points{j}=text(x,y,num2str(neuron_idx(j)));
        set(handles.points{j},'Color',handles.colorset(neuron_idx(j),:));
        set(handles.points{j},'HorizontalAlignment','center');
        set(handles.points{j},'ButtonDownFcn','whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
        set(handles.points{j},'fontsize',handles.Font_sizeMenuItem);
    end
    
end

%ROI

if ~isempty(handles.ROIposition{handles.current_volume,1})
    rect=handles.ROIposition{handles.current_volume,1};
    handles.ROI{handles.current_volume,handles.current_slice}=rectangle('Curvature', [0 0],'Position',rect,'EdgeColor','y');
    
elseif ~isempty(handles.ROIposition{max(handles.current_volume-1,1),1})
    rect=handles.ROIposition{max(handles.current_volume-1,1),1};
    handles.ROI{handles.current_volume,1}=rectangle('Curvature', [0 0],'Position',rect,'EdgeColor','y');
    handles.ROIposition{handles.current_volume,1}=rect;
end

%online pixel intensity calculation
handles.hp=impixelinfo;

set(handles.hp,'Position',[0 44+handles.axis_height+2,300,20]);

guidata(hObject,handles);


% --------------------------------------------------------------------
function Enhance_contrastMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to Enhance_contrastMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
checked=get(hObject,'Checked');
if strcmp(checked,'off')
    set(hObject,'Checked','on');
    handles.contrast_enhancement=1;
else
    set(hObject,'Checked','off');
    handles.contrast_enhancement=0;
end

guidata(hObject,handles);


% --------------------------------------------------------------------
function Font_sizeMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to Font_sizeMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
dlg_title='Font Size';
prompt={'Font Size'};
num_lines=1;
marks=inputdlg(prompt,dlg_title,num_lines);
handles.Font_sizeMenuItem=str2num(marks{1}(1,:));
set(handles.slider1,'Value',handles.current_volume);

set(handles.text1, 'String',strcat(num2str(handles.current_volume),'/',num2str(handles.volumes),' volumes; '));
axes(handles.axes1);
cla;
if handles.contrast_enhancement
    img=imagesc(contrast_enhancement(handles.img_stack{handles.current_volume,1}(:,:)),[handles.low handles.high]);
else
    img=imagesc(handles.img_stack{handles.current_volume,1}(:,:),[handles.low handles.high]);
end
colormap(gray);
set(handles.axes1, 'Visible', 'off');
set(img,'ButtonDownFcn', 'whole_brain_imaging(''ButtonDown_Callback'',gcbo,[],guidata(gcbo))');

if ~isempty(handles.neuronal_position{handles.current_volume,1})
    
    N=size((handles.neuronal_position{handles.current_volume,1}),2);
    neuron_idx=handles.neuronal_idx{handles.current_volume,1};
    handles.colorset=varycolor(max(neuron_idx)); %a bug fixed by fanchan
    for j=1:N
        
        x=handles.neuronal_position{handles.current_volume,1}(1,j);
        y=handles.neuronal_position{handles.current_volume,1}(2,j);
        hold on; 
        
        handles.points{j}=text(x,y,num2str(neuron_idx(j)));
        set(handles.points{j},'Color',handles.colorset(neuron_idx(j),:));
        set(handles.points{j},'HorizontalAlignment','center');
        set(handles.points{j},'ButtonDownFcn','whole_brain_imaging(''ButtonDownPoint_Callback'',gcbo,[],guidata(gcbo))');
        set(handles.points{j},'fontsize',handles.Font_sizeMenuItem);
        
    end
    
end
handles.hp=impixelinfo;

set(handles.hp,'Position',[0 44+handles.axis_height+2,300,20]);
guidata(hObject,handles);

% --------------------------------------------------------------------
function Tracking_allMenuItem_Callback(hObject, eventdata, handles,fil,worm_area_est,sizethresh,img_bkg)
% hObject    handle to Tracking_allMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
answer = inputdlg({'Start Frame', 'Frames'}, '', 1, {num2str(handles.current_volume),num2str(500)});
istart = str2double(answer{1});
iend = istart+str2double(answer{2});
    fil=handles.fil;
    sizethresh=handles.sizethresh;
    worm_area_est=handles.worm_area_est;
    img_bkg=handles.img_bkg;

for i = istart:iend
    if i==iend
         
    imgStack=handles.img_stack{i,1};
    preheadtail=handles.neuronal_position{i,1};

    %preheadandtail
    %fil and area
    
        if handles.contrast_enhancement
        

            [nextheadtail,cv2i] = identify_centerline(contrast_enhancement(imgStack),preheadtail,fil,worm_area_est,sizethresh,img_bkg);
         else
        
            [nextheadtail,cv2i] = identify_centerline(imgStack,preheadtail,fil,worm_area_est,sizethresh,img_bkg);
        end
        handles.cv2i{i,1}=cv2i;
        handles.neuronal_idx{i,1}=[1 2];
    else 
        imgStack=handles.img_stack{i,1};
        preheadtail=handles.neuronal_position{i,1};

    
        if handles.contrast_enhancement
        

            [nextheadtail,cv2i] = identify_centerline(contrast_enhancement(imgStack),preheadtail,fil,worm_area_est,sizethresh,img_bkg);
         else
        
            [nextheadtail,cv2i] = identify_centerline(imgStack,preheadtail,fil,worm_area_est,sizethresh,img_bkg);
         end
    

    handles.neuronal_idx{i,1}=[1 2];
    handles.neuronal_position{i+1,1}=nextheadtail;
    handles.cv2i{i,1}=cv2i;
    end
end

msgbox('completed');
guidata(hObject,handles);



% --- Executes on button press in pushbutton4.
function pushbutton4_Callback(hObject, eventdata, handles)
load('data1.mat');
handles.fil=fil;
handles.img_bkg=img_bkg;
handles.sizethresh=sizethresh;
handles.worm_area_est = worm_area_est
guidata(hObject,handles);
% hObject    handle to pushbutton4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function weird_Callback(hObject, eventdata, handles)
% hObject    handle to weird (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% hObject    handle to Tracking_allMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
answer = inputdlg({'Start Frame', 'Frames'}, '', 1, {num2str(handles.current_volume),num2str(500)});
istart = str2double(answer{1});
iend = istart+str2double(answer{2});
if iend > handles.volumes
    iend = handles.volumes;
end

handles.weird(istart:iend)=1;
guidata(hObject,handles);
% handles.weird=zeros(1,handles.volumes);
% handles.forward=zeros(1,handles.volumes);
% handles.backward=zeros(1,handles.volumes);
% handles.beforeturn=zeros(1,handles.volumes);
% handles.beforereversal=zeros(1,handles.volumes);

% --------------------------------------------------------------------
function forward_Callback(hObject, eventdata, handles)
% hObject    handle to forward (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
answer = inputdlg({'Start Frame', 'Frames'}, '', 1, {num2str(handles.current_volume),num2str(500)});
istart = str2double(answer{1});
iend = istart+str2double(answer{2});
if iend > handles.volumes
    iend = handles.volumes;
end
handles.forward(istart:iend)=1;
guidata(hObject,handles);
% handles.weird=zeros(1,handles.volumes);
% handles.forward=zeros(1,handles.volumes);
% handles.backward=zeros(1,handles.volumes);
% handles.beforeturn=zeros(1,handles.volumes);
% handles.beforereversal=zeros(1,handles.volumes);


% --------------------------------------------------------------------
function backward_Callback(hObject, eventdata, handles)
% hObject    handle to backward (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
answer = inputdlg({'Start Frame', 'Frames'}, '', 1, {num2str(handles.current_volume),num2str(500)});
istart = str2double(answer{1});
iend = istart+str2double(answer{2});
if iend > handles.volumes
    iend = handles.volumes;
end

handles.backward(istart:iend)=1;
guidata(hObject,handles);

% --------------------------------------------------------------------
function beforeturn_Callback(hObject, eventdata, handles)
% hObject    handle to beforeturn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
answer = inputdlg({'Start Frame', 'Frames'}, '', 1, {num2str(handles.current_volume),num2str(500)});
istart = str2double(answer{1});
iend = istart+str2double(answer{2});
if iend > handles.volumes
    iend = handles.volumes;
end

handles.beforeturn(istart:iend)=1;
guidata(hObject,handles);
% handles.weird=zeros(1,handles.volumes);
% handles.forward=zeros(1,handles.volumes);
% handles.backward=zeros(1,handles.volumes);
% handles.beforeturn=zeros(1,handles.volumes);
% handles.beforereversal=zeros(1,handles.volumes);



% --------------------------------------------------------------------
function beforereversal_Callback(hObject, eventdata, handles)
% hObject    handle to beforereversal (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
answer = inputdlg({'Start Frame', 'Frames'}, '', 1, {num2str(handles.current_volume),num2str(500)});
istart = str2double(answer{1});
iend = istart+str2double(answer{2});
if iend > handles.volumes
    iend = handles.volumes;
end

handles.beforeturn(istart:iend)=1;
guidata(hObject,handles);
% handles.weird=zeros(1,handles.volumes);
% handles.forward=zeros(1,handles.volumes);
% handles.backward=zeros(1,handles.volumes);
% handles.beforeturn=zeros(1,handles.volumes);
% handles.beforereversal=zeros(1,handles.volumes);




% --------------------------------------------------------------------
function identify_Callback(hObject, eventdata, handles)
% hObject    handle to identify (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

sh=handles.current_volume;
if handles.forward(sh)==1 & sum(handles.forward(sh)+handles.weird(sh)+handles.backward(sh)+handles.beforeturn(sh)+handles.beforereversal(sh))==1
    disp('forward ');
elseif   handles.weird(sh)==1& sum(handles.forward(sh)+handles.weird(sh)+handles.backward(sh)+handles.beforeturn(sh)+handles.beforereversal(sh))==1
    disp('weird ');
elseif handles.backward(sh)==1& sum(handles.forward(sh)+handles.weird(sh)+handles.backward(sh)+handles.beforeturn(sh)+handles.beforereversal(sh))==1
    disp('backward ');
elseif handles.beforeturn(sh)==1& sum(handles.forward(sh)+handles.weird(sh)+handles.backward(sh)+handles.beforeturn(sh)+handles.beforereversal(sh))==1
    disp('beforeturn ');
elseif  handles.beforereversal(sh)==1& sum(handles.forward(sh)+handles.weird(sh)+handles.backward(sh)+handles.beforeturn(sh)+handles.beforereversal(sh))==1
    disp('reversal ');
elseif sum(handles.forward(sh)+handles.weird(sh)+handles.backward(sh)+handles.beforeturn(sh)+handles.beforereversal(sh))>1
    disp('not only one behaviour');
else
    disp("?");
end


guidata(hObject,handles);



% --- Executes during object creation, after setting all properties.
function text1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to text1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called
