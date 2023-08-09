function test_optFlowErr
clearvars;
close all;

dir0 = 'C:/Users/wus1/Projects/2023/opt-flow-evaluation/dataset/fpts';
tagA = 'fpt-cv_gpu-';
tagB = 'fpt-nv_vpi-';
goodStatusFlagA = 1;
goodStatusFlagB = 0;
nExpectedRows=100;

fn2=96;
%read init frame
xA0 = readOneFrmFlow( 0, dir0, tagA, goodStatusFlagA, nExpectedRows );
xB0 = readOneFrmFlow( 0, dir0, tagB, goodStatusFlagB, nExpectedRows );

%find correspondence from init frame
tab = nan(nExpectedRows,2);  %[rowNumInA, correspondRowNumInB]
for i=1:nExpectedRows
    %start point for A
    pA = xA0(i, 2:3);

    %start point for B
    [idxB] = findCorrespondPt( xB0, pA);
    tab(i,:) = [i, idxB];
end

%read 1-fn2 frames and they are allinge row-by-row
xA = readMultiFrmFlow( 1, fn2, dir0, tagA, [], goodStatusFlagA, nExpectedRows);
xB = readMultiFrmFlow( 1, fn2, dir0, tagB, tab, goodStatusFlagB, nExpectedRows);

%compute frame error
vErrMean=nan(1,fn2);
vErrStd=nan(1,fn2);
vGoodPts=nan(1,fn2);
vOutliers=nan(1,fn2);
traceContinues = ones(1, nExpectedRows);
for fn=1:fn2
    
    fprintf('----------fn=%d-----------\n',fn);

    z = nan(1, nExpectedRows);
    A = xA{fn};
    B = xB{fn};
    nOutliers = 0;
    for k=1:nExpectedRows
        if 1==traceContinues(k)
            if A(k, 4)==1 && B(k,4)==1
                dd = norm( A(k,2:3) -B(k,2:3) );
                if dd<5
                    z(k) = dd;
                else
                    nOutliers = nOutliers + 1;
                end
            else
                traceContinues(k) = 0;
            end
        end
    end
    I = ~isnan(z);
    zz = z(I);
    vErrMean(fn) = mean(zz);
    vErrStd(fn) = std(zz);
    vGoodPts(fn) = length(zz);
    vOutliers(fn) = nOutliers;
end

disp(vErrMean);

fprintf('vErrStd=\n');
disp(vErrStd);

fprintf('vGoodPts=\n');
disp(vGoodPts);

fprintf('vOutliers=\n');
disp(vOutliers);

x = (1:fn2);

figure 
hold on;
yyaxis left
errorbar(x, vErrMean, vErrStd/2, 'LineWidth',3);
xlim([0,100])
ylim([-0.2,1.0])
ylabel( 'averaged accumulated corresponding feature point distance (mean,std)', 'FontSize', 18 );

yyaxis right
plot(x, vGoodPts, 'LineWidth',3 );
xlim([0,100]);
ylim([60,100]);
xlabel( 'video frame number', 'FontSize', 18 );
ylabel( '# of feature points continousely tracked by both methods','FontSize', 18 );

a = get(gca,'XTickLabel');
set(gca,'XTickLabel',a,'FontName','Times','fontsize',20)
b = get(gca,'YTickLabel');
set(gca,'YTickLabel',b,'FontName','Times','fontsize',20)

end


function  [idx] = findCorrespondPt( x, p)
% p, 1 x 2, [x,y]
% x, n x 4, [id, featurePt.x, featurePt.y, status]
idx = -1;  %not found flag
thd = 0.1;
[nPts, four] = size( x );
assert( four == 4 );
for i=1:nPts
    if abs(p(1) - x(i,2)) < thd  && abs(p(2) - x(i,3)) < thd
        idx = i;
        break;
    end
end
end


function x = readMultiFrmFlow( fn1, fn2, dir0, tag, corrTab, goodStatusFlag, nExpectedRows )
%-------------------------------------------------------------------------------------
%fn1, start frame #
%fn2, end frame #
%dir0, data folder dir
%tag,  file name tag
%corrTab, teh cooresponding table between flow calculation method A and method B
%goodStatusFlag, for cpu and cv-gpu: 1 is good status, 0 is bad status,
%                for nv-vpi:  0 is good status, 1 is bad status,
%nExpectedRows, expected rows in data file
%-------------------------------------------------------------------------------------
assert( fn1<=fn2 );
x = cell(fn2, 1);
for i=fn1:fn2
    raw = readOneFrmFlow( i, dir0, tag, goodStatusFlag, nExpectedRows );
    if isempty(corrTab )
        x{i} = raw;
    else
        [m,n] = size(raw);
        [mTab,two] = size(corrTab);
        reOrdered = nan(m,n);
        for j =1:mTab
            assert( j==corrTab(j, 1) );
            k = corrTab(j, 2);
            fprintf('j=%d, k=%d\n', j, k);
            if k>0
                reOrdered(j, :) = raw(k, :);
            end
        end
        x{i} = reOrdered;
    end
end
end

