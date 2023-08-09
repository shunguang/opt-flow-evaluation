function main_compMultiFrameErr_wo_gt
clearvars;
close all;

caseFlag=3;
if caseFlag==1
    seqName = 'cars.mov';
    dir0 = 'C:/Users/wus1/Projects/2023/opt-flow-evaluation/results/without-gt/out-cars-multiFrm';
    fn2=94;
elseif caseFlag==2
    seqName = 'car.mov';
    dir0 = 'C:/Users/wus1/Projects/2023/opt-flow-evaluation/results/without-gt/out-car-multiFrm';
    fn2=98;
else
    seqName = 'street.mov';
    dir0 = 'C:/Users/wus1/Projects/2023/opt-flow-evaluation/results/without-gt/out-street-multiFrm';
    fn2=98;
end
outlierThd = 5;

tagA = 'multi-cv-cpu';
tagB = 'multi-cv-gpu';
tagC = 'multi-nv-vpi';
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

vGoodPts_A=nan(1,fn2);
vGoodPts_B=nan(1,fn2);
vGoodPts_C=nan(1,fn2);

myTitle = [seqName, ', outlier distance thd=', num2str(outlierThd)];

%read the start frm's feature points
A = readMultiFrmFlow( 0, dir0, tagA, goodStatusFlagA, nExpectedRows );
B = readMultiFrmFlow( 0, dir0, tagB, goodStatusFlagA, nExpectedRows );
C = readMultiFrmFlow( 0, dir0, tagC, goodStatusFlagB, nExpectedRows );
%check: the start frame (x,y) should be the same with each other
epsAB = sum(sum(abs(A(:,2:3)-B(:,2:3))));
epsAC = sum(sum(abs(A(:,2:3)-C(:,2:3))));
epsBC = sum(sum(abs(B(:,2:3)-C(:,2:3))));
assert( epsAB < eps );
assert( epsAC < eps );
assert( epsBC < eps );

for fn=1:fn2
    A = readMultiFrmFlow( fn, dir0, tagA, goodStatusFlagA, nExpectedRows );
    B = readMultiFrmFlow( fn, dir0, tagB, goodStatusFlagA, nExpectedRows );
    C = readMultiFrmFlow( fn, dir0, tagC, goodStatusFlagB, nExpectedRows );
    %A, n x 4, [id, featurePt.x, featurePt.y, status]
    %B, n x 4, [id, featurePt.x, featurePt.y, status]
    %C, n x 4, [id, featurePt.x, featurePt.y, status]
    vGoodPts_A(fn) = sum(A(:,4) == 1);
    vGoodPts_B(fn) = sum(B(:,4) == 1);
    vGoodPts_C(fn) = sum(C(:,4) == 1);
    
    [n, meanDist, stdDist] = calDistMulti(A,B,outlierThd);
    vGoodPts_AB (fn) = n;
    d_mean_AB(fn) = meanDist;
    d_std_AB(fn) = stdDist;

    [n, meanDist, stdDist] = calDistMulti(A,C,outlierThd);
    vGoodPts_AC(fn) = n;
    d_mean_AC(fn) = meanDist;
    d_std_AC(fn) = stdDist;

    [n, meanDist, stdDist] = calDistMulti(B,C,outlierThd);
    vGoodPts_BC(fn) = n;
    d_mean_BC(fn) = meanDist;
    d_std_BC(fn) = stdDist;   
end

    x = (1:fn2);

    figure
    subplot(2,1,1)
    hold on;
    errorbar(x, d_mean_AB, d_std_AB/2, 'r+-', 'LineWidth',3);
    errorbar(x, d_mean_AC, d_std_AC/2, 'g+-', 'LineWidth',3);
    errorbar(x, d_mean_BC, d_std_BC/2, 'b+-', 'LineWidth',3);
    legend('CV-CPU vs. CV-CUDA', 'CV-CPU vs. NV-VPI', 'CV-CUDA vs. NV-VPI');
    xlim([0,100])
    %ylim([-0.05,0.3])
    ylabel( 'tracked feature points distance(mean,std)', 'FontSize', 18 );
    ax = gca;
    ax.XAxis.FontSize = 16;
    ax.YAxis.FontSize = 16;
    ax.FontWeight = 'bold';
    %a = get(gca,'XTickLabel');
    %set(gca,'XTickLabel',a,'FontName','Times','fontsize',15)
    %b = get(gca,'YTickLabel');
    %set(gca,'YTickLabel',b,'FontName','Times','fontsize',15)
    title( myTitle, 'fontsize',20);
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
    ax.XAxis.FontSize = 16;
    ax.YAxis.FontSize = 16;
    ax.FontWeight = 'bold';
    hold off;

end


