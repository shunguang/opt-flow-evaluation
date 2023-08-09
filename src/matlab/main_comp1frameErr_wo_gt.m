function main_comp1frameErr_wo_gt
clearvars;
close all;

if 0
    seqName = "cars.mov";
    dir0 = 'C:/Users/wus1/Projects/2023/opt-flow-evaluation/results/without-gt/out-cars-1frm';
    fn2=95;
elseif 0
    seqName = "car.mov";
    dir0 = 'C:/Users/wus1/Projects/2023/opt-flow-evaluation/results/without-gt/out-car-1frm';
    fn2=99;
elseif 1
    seqName = "streetOverlooking seq.";
    dir0 = 'C:/Users/wus1/Projects/2023/opt-flow-evaluation/results/without-gt/out-street-1frm';
    fn2=99;
end

tagA = 'fpt-cv-cpu';
tagB = 'fpt-cv-gpu';
tagC = 'fpt-nv-vpi';
goodStatusFlagA = 1;
goodStatusFlagB = 0;
nExpectedRows=100;


d_mean_AB=nan(1,fn2);
d_std_AB=nan(1,fn2);

d_mean_AC=nan(1,fn2);
d_std_AC=nan(1,fn2);

vGoodPts_AB=nan(1,fn2);
vGoodPts_AC=nan(1,fn2);
vGoodPts_BC=nan(1,fn2);
d_mean_BC=nan(1,fn2);
d_std_BC=nan(1,fn2);
outlierThd = 0.5;

vGoodPts_A=nan(1,fn2);
vGoodPts_B=nan(1,fn2);
vGoodPts_C=nan(1,fn2);

for fn=1:fn2
    A = readOneFrmFlow_v2( fn, dir0, tagA, goodStatusFlagA, nExpectedRows );
    B = readOneFrmFlow_v2( fn, dir0, tagB, goodStatusFlagA, nExpectedRows );
    C = readOneFrmFlow_v2( fn, dir0, tagC, goodStatusFlagB, nExpectedRows );
    %A, n x 6, [id, prevFrmFeaturePt.x, prevFrmFeaturePt.y, currFrmFeaturePt.x, currFrmFeaturePt.y, status]
    %B, n x 6, [id, prevFrmFeaturePt.x, prevFrmFeaturePt.y, currFrmFeaturePt.x, currFrmFeaturePt.y, status]
    %C, n x 6, [id, prevFrmFeaturePt.x, prevFrmFeaturePt.y, currFrmFeaturePt.x, currFrmFeaturePt.y, status]
    vGoodPts_A(fn) = sum(A(:,6) == 1);
    vGoodPts_B(fn) = sum(B(:,6) == 1);
    vGoodPts_C(fn) = sum(C(:,6) == 1);

    assert( sum(sum(abs(A(:,2:3) - B(:,2:3))))==0 );
    assert( sum(sum(abs(A(:,2:3) - C(:,2:3))))==0 );
    assert( sum(sum(abs(B(:,2:3) - C(:,2:3))))==0 );

    [n, meanDist, stdDist] = calDist(A,B,outlierThd);
    vGoodPts_AB (fn) = n;
    d_mean_AB(fn) = meanDist;
    d_std_AB(fn) = stdDist;

    [n, meanDist, stdDist] = calDist(A,C,outlierThd);
    vGoodPts_AC(fn) = n;
    d_mean_AC(fn) = meanDist;
    d_std_AC(fn) = stdDist;

    [n, meanDist, stdDist] = calDist(B,C,outlierThd);
    vGoodPts_BC(fn) = n;
    d_mean_BC(fn) = meanDist;
    d_std_BC(fn) = stdDist;
end

x = (1:fn2);

figure
subplot(2,1,1)
hold on;
errorbar(x, d_mean_AB, d_std_AB/2, 'r-', 'LineWidth',3);
errorbar(x, d_mean_AC, d_std_AC/2, 'g-', 'LineWidth',3);
errorbar(x, d_mean_BC, d_std_BC/2, 'b-', 'LineWidth',3);
legend('CV-CPU vs. CV-CUDA', 'CV-CPU vs. NV-VPI', 'CV-CUDA vs. NV-VPI');
xlim([0,100])
%ylim([-0.05,0.3])
ylabel( 'tracked feature points distance(mean,std)', 'FontSize', 18 );
ax = gca;
ax.XAxis.FontSize = 20;
ax.YAxis.FontSize = 20;
ax.FontWeight = 'bold';
%a = get(gca,'XTickLabel');
%set(gca,'XTickLabel',a,'FontName','Times','fontsize',15)
%b = get(gca,'YTickLabel');
%set(gca,'YTickLabel',b,'FontName','Times','fontsize',15)
title(seqName, 'fontsize',20);
hold off;

%    figure
subplot(2,1,2)
hold on
plot(x, vGoodPts_A, 'r+', 'LineWidth',3 );
plot(x, vGoodPts_B, 'go', 'LineWidth',3 );
plot(x, vGoodPts_C, 'bs', 'LineWidth',3 );
%plot(x, vGoodPts_AB, 'kx', 'LineWidth',3 );
%plot(x, vGoodPts_AC, 'm*', 'LineWidth',3 );
%plot(x, vGoodPts_BC, 'c^', 'LineWidth',3 );
%legend( 'CV-CPU', 'CV-GPU', 'NV-VPI', 'CV-CPU & CV-CUDA', 'CV-CPU & NV-VPI', 'CV-CUDA & NV-VPI');
legend( 'CV-CPU', 'CV-GPU', 'NV-VPI');
xlim([0,100]);
ymin = min([min(vGoodPts_A), min(vGoodPts_B), min(vGoodPts_C)])-5;
ymax = max([max(vGoodPts_A), max(vGoodPts_B), max(vGoodPts_C)])+5;
ylim([ymin, ymax]);
xlabel( 'video frame number', 'FontSize', 18 );
ylabel( '# of feature points tracked','FontSize', 18 );

%a = get(gca,'XTickLabel');
%set(gca,'XTickLabel',a,'FontName','Times','fontsize',15)
%b = get(gca,'YTickLabel');
%set(gca,'YTickLabel',b,'FontName','Times','fontsize',15)

ax = gca;
ax.XAxis.FontSize = 20;
ax.YAxis.FontSize = 20;
ax.FontWeight = 'bold';
hold off;

end


