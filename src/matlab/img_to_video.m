function img_to_video

warning off all
if 0
    outputVideoFile = 'c:/temp/of-nv-vpi-street.mov';
    dirIn = 'C:\Users\wus1\Projects\2023\opt-flow-evaluation\results\without-gt\out-street-multiFrm-img';
    tag ='cv-cpu-OF_';
    nFrms=99;
elseif 0
    outputVideoFile = 'c:/temp/of-nv-vpi-cars.mov';
    dirIn = 'C:\Users\wus1\Projects\2023\opt-flow-evaluation\results\without-gt\out-cars-multiFrm-img';
    tag ='cv-cpu-OF_';
    nFrms=95;
elseif 0
    outputVideoFile = 'c:/temp/of-nv-vpi-car.mov';
    dirIn = 'C:\Users\wus1\Projects\2023\opt-flow-evaluation\results\without-gt\out-car-multiFrm-img';
    tag ='cv-cpu-OF_';
    nFrms=99;
elseif 1
    outputVideoFile = 'C:\Users\wus1\Projects\2023\opt-flow-evaluation\dataset\street.mov';

    dirIn = 'C:\Users\wus1\Projects\2023\opt-flow-evaluation\dataset\street\img';
    tag ='ori-frm_';
    nFrms=99;   
end

fprintf('The output video file name is: %s\n', outputVideoFile);
fprintf('I am running, please wait ...');

video = VideoWriter(outputVideoFile, 'MPEG-4'); 
video.FrameRate = 10;

open(video); %open the file for writing

for fn=0:nFrms
    f = [dirIn, '\',  tag, num2str(fn, '%04d'), '.png'];
    I = imread(f);
    I = insertText(I, [10, 10], fn, 'AnchorPoint','LeftTop', 'FontSize', 20  );
    writeVideo(video,I);      
end
close( video );
fprintf('\n*----------------*\n');
fprintf('* Job well done! *\n');
fprintf('*----------------*\n');
%eof

