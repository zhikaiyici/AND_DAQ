function event = Preprocess(listFileName, timeFileName, fCalib)
% 去除编号重复的无效事件，取Time和List数据的交集

% edepData = importdata('LIST0.txt');
% edepData = importdata(listFileName);
% edepData = edepData.data;
edepData = importdata(listFileName);

% timeData = importdata('TIME0.txt');
% timeData = importdata(timeFileName);
% timeData = timeData.data;
timeData = importdata(timeFileName);

time = timeData(2:2:end);
timeID = timeData(1:2:end);
clear timeData;
[~, uniqueTimeIndex] = unique(timeID,'stable');
duplicateTimeIndex = setdiff((1:size(timeID, 1))',uniqueTimeIndex);
clear uniqueTimeIndex;

time = [timeID, time];
clear timeID;
time(duplicateTimeIndex,:) = [];

edep = edepData;
edep(1:5:end,:) = [];
edep = sqrt(edep(:,1:2:end) .* edep(:,2:2:end));
edep = fCalib(edep);
edep(edep < 200) = 0;
edepID = edepData(1:5:end, 1);
clear edepData;
edep = mat2cell(edep, 4 .* ones(size(edep,1) ./ 4, 1), size(edep,2));
[~, uniqueEdepIndex] = unique(edepID,'stable');
duplicateEdepIndex = setdiff((1:size(edepID, 1))',uniqueEdepIndex);
clear uniqueEdepIndex;
edep = [num2cell(edepID), edep];
clear edepID;
edep(duplicateEdepIndex,:) = [];

eventID = intersect(time(:,1), cell2mat(edep(:,1)), 'stable');
[~, dropedTimeIndex] = setdiff(time(:,1), eventID);
[~, dropedEdepIndex] = setdiff(cell2mat(edep(:,1)), eventID);
clear eventID;

time(dropedTimeIndex, :) = [];
edep(dropedEdepIndex, :) = [];

event = [num2cell(time), edep(:,2)];
clearvars -except event;

% name = listFileName(10:end);
% name = strrep(name, '.txt', '');
% while contains(name, ' ')
%     name = strrep(name, ' ', '');
% end
% 
% clearvars -except event name;
% save(['event', name, '.mat'], 'event');

end
