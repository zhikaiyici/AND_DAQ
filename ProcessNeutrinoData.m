clear
dirName = 'sim_1600';
cd(dirName);
% dataFolder = pwd;
name = ls;
cd ..
name = name(3:end,:);
fileNum = size(name, 1) / 2;
listName = name(1:fileNum,:);
timeName = name(fileNum + 1:end,:);
IBDEvent = cell(fileNum, 5);
fCalibD = @(ADC) ADC .* 1.734 - 194.3; % 刻度探测器 E(keV) = 1.734 * ADC - 194.3
% fCalibS = @(ADC) ADC .* 1.702 - 281.6; % 刻度模拟信号发生器 E(keV) = 1.702 * ADC - 281.6 % 1900
fCalibS = @(ADC) ADC .* 2.015 - 323.3; % 刻度模拟信号发生器 E(keV) = 2.015 * ADC - 323.3 % 1600
for ii = 1:fileNum
    listFileName = [dirName, '\', listName(ii,:)];
    timeFileName = [dirName, '\', timeName(ii,:)];
    name = listFileName(10:end);
    name = strrep(name, '.txt', '');
    while contains(name, ' ')
        name = strrep(name, ' ', '');
    end
    event = Preprocess(listFileName, timeFileName, fCalibS);    
    temp = IBDSelection(event);
%     IBDEvent(ii,:) = {name, temp, size(temp, 1)};
    IBDEvent(ii,:) = {name, temp, size(temp, 1), size(event,1), 100 .* size(temp, 1) ./2 ./ 9000};
end
save('IBDEvent_Sim_1600.mat', 'IBDEvent');