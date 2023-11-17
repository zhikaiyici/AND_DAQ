function IBDEvent = IBDSelection(event)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%          |          瞬时信号         |           延迟信号
% 甄别项目  |         甄别条件          |           甄别条件
% 预甄别    |      Eth > 200 keV       |        Eth > 200 keV
% Ntrigger |     2 ≤ Ntrigger ≤ 16   |    2 ≤ Ntrigger ≤ 16
% Etotal   | 2.5 MeV < Etotal ≤ 7 MeV | 2.8 MeV < Etotal ≤ 8 MeV
% E1st     |  1 MeV < E1st ≤ 6.5 MeV  |  0.5 MeV < E1st ≤ 7 MeV
% E2nd     |       E2nd < 520 keV      |       E2nd ≤ 3 MeV
% E3rd     |             --            |       E3rd ≤ 2 MeV
% E4th     |             --            |       E4th ≤ 1 MeV
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
edep = cell2mat(event(:,3));
edep = ReshapeDataMatrix(4, edep);
edep(edep < 200) = 0;
dropedEvent = (permute(sum(sum(edep > 0)), [3, 1, 2]) < 2);
event(dropedEvent,:) = []; % 去除触发数量小于2的事件

edep = cell2mat(event(:,3));
edep = ReshapeDataMatrix(4, edep);
% muonEvent = (permute(sum(sum(edep > 0)), [3, 1, 2]) > 2);
totalEdep = permute(sum(sum(edep)), [3, 1, 2]);
muon = (totalEdep > 10000);
% muonEvent = event(muon,:);
muonTime = cell2mat(event(muon, 2));
muonGeneratedTime = muonTime + 300;
% muonTime = muonTime - 0.02;
dropedNum = [];
for ii = 1:10
    muonGenerated = [false(ii,1); muon(1:end - ii)];
    muonGeneratedEvent = event(muonGenerated,:);
    tempMounTime = muonGeneratedTime(1:size(muonGeneratedEvent, 1));
    dropedGenerated = cell2mat(muonGeneratedEvent(:,2)) < tempMounTime;
    dropedNum = unique([dropedNum; cell2mat(muonGeneratedEvent(dropedGenerated, 1))]);
end
[~,dropedIndex] = intersect(cell2mat(event(:,1)), dropedNum);
% event(muon,:) = [];
event(dropedIndex,:) = []; %去除u子事件之后300us内的事件

edep = cell2mat(event(:,3));
edep = ReshapeDataMatrix(4, edep);
totalEdep = permute(sum(sum(edep)), [3, 1, 2]);
dropedEvent = (totalEdep < 2500 | totalEdep > 8000);
event(dropedEvent,:) = []; % 去除总能量小于2.5MeV、大于8MeV的事件

edep = cell2mat(event(:,3));
edep = reshape(edep',[16, numel(edep) ./ 16])';
edepSorted = sort(edep, 2, 'descend');
dropedEvent = (edepSorted(:,1) < 500);
event(dropedEvent,:) = []; % 去除E1st小于0.5MeV的事件

time = cell2mat(event(:,2));
timeGap = time(2:end) - time(1:end - 1);
dropedEvent = (timeGap > 300 | timeGap < 8);
index = find(dropedEvent == 0);
index = unique([index; index + 1]);
dropedEvent = [dropedEvent; true(1)];
dropedEvent(index) = 0;
event(dropedEvent,:) = []; % 去时间差大于300us或小于8us的除相邻事件

time = cell2mat(event(:,2));
timeGap = time(2:end) - time(1:end - 1);
prompt = (timeGap < 200);
promptEvent = event(prompt,:);
edep = cell2mat(promptEvent(:,3));
edep = reshape(edep',[16, numel(edep) ./ 16])';
edepSorted = sort(edep, 2, 'descend');
dropedEvent = (sum(edepSorted, 2) > 7000 | edepSorted(:,1) > 6500 | edepSorted(:,2) > 520);
% index = find(dropedEvent == 0);
promptEvent(dropedEvent,:) = []; % Etotoal > 7MeV、E1st > 6.5MeV、E2nd > 520keV的瞬时事件

edep = cell2mat(promptEvent(:,3));
edep = reshape(edep',[16, numel(edep) ./ 16])';
[e1st, iE1st] = max(edep, [], 2);
edep(edep == e1st) = 0;
[~, iE2nd] = max(edep, [], 2);
[r1st, c1st] = ind2sub(4, iE1st);
[r2nd, c2nd] = ind2sub(4, iE2nd);
dropedEvent = (abs(r1st - r2nd) > 2 | abs(c1st - c2nd) > 2); 
% promptEvent(dropedEvent,:) = []; % 去除E1st和E2nd不相邻的事件

[~, reservedIndex] = intersect(cell2mat(event(:,1)), cell2mat(promptEvent(:,1)), 'stable');
reservedEventIndex = unique([reservedIndex; reservedIndex + 1]);
event = event(reservedEventIndex,:);

time = cell2mat(event(:,2));
timeGap = time(2:end) - time(1:end - 1);
prompt = (timeGap < 200);
delayedEvent = event;
delayedEvent(prompt,:) = [];

edep = cell2mat(delayedEvent(:,3));
edep = reshape(edep',[16, numel(edep) ./ 16])';
edepSorted = sort(edep, 2, 'descend');
dropedEvent = (sum(edepSorted, 2) < 2800 |edepSorted(:,1) > 7000 | edepSorted(:,2) > 3000 | edepSorted(:,3) > 2000 | edepSorted(:,4) > 1000);
delayedEvent(dropedEvent,:) = []; % 去除Etotal < 2.8MeV、E1st > 7MeV、E2nd > 3MeV、E3rd > 2 MeV、E4th > 1MeV的延迟事件

[~, reservedIndex] = intersect(cell2mat(event(:,1)), cell2mat(delayedEvent(:,1)), 'stable');

reservedEventIndex = unique([reservedIndex - 1; reservedIndex]);
IBDEvent = event(reservedEventIndex,:);

end
