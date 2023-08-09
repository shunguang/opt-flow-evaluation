function main_comp1frameErr_w_gt
clearvars;
close all;

seqName = "Class Room";

nTotExps = 1;
dir0{1} = 'C:/Users/wus1/Projects/2023/opt-flow-evaluation/results/with-gt/classroom-exp2-winSz3';
tagA{1} = 'classRoom-result-cv-cpu.txt';
tagB{1} = 'classRoom-result-cv-gpu.txt';
tagC{1} = 'classRoom-result-nv-vpi.txt';

goodStatusFlagAB = 1;
goodStatusFlagC = 0;

vGoodPts_A=nan(1,nTotExps);
vGoodPts_B=nan(1,nTotExps);
vGoodPts_C=nan(1,nTotExps);

for i=1:nTotExps
    A = readOneFrmFlow_gt( dir0{i}, tagA{i}, goodStatusFlagAB);
    B = readOneFrmFlow_gt( dir0{i}, tagB{i}, goodStatusFlagAB);
    C = readOneFrmFlow_gt( dir0{i}, tagC{i}, goodStatusFlagC);

    %A:%x, n x 8, [id, frame1.x, frame1.y, gt_frame2.x, gt_frame2.y, est_frame2.x, est_frame2.y, status]
    %B:%x, n x 8, [id, frame1.x, frame1.y, gt_frame2.x, gt_frame2.y, est_frame2.x, est_frame2.y, status]
    %C:%x, n x 8, [id, frame1.x, frame1.y, gt_frame2.x, gt_frame2.y, est_frame2.x, est_frame2.y, status]

    vGoodPts_A(i) = sum(A(:,8) == 1);
    vGoodPts_B(i) = sum(B(:,8) == 1);
    vGoodPts_C(i) = sum(C(:,8) == 1);

    assert( sum(sum(abs(A(:,2:3) - B(:,2:3))))==0 );
    assert( sum(sum(abs(A(:,2:3) - C(:,2:3))))==0 );
    assert( sum(sum(abs(B(:,2:3) - C(:,2:3))))==0 );
    assert( sum(sum(abs(A(:,4:5) - B(:,4:5))))==0 );
    assert( sum(sum(abs(A(:,4:5) - C(:,4:5))))==0 );
    assert( sum(sum(abs(B(:,4:5) - C(:,4:5))))==0 );

    [m,n] = size(A);
    x = A(:,1);
    errA = calErr(A)
    errB = calErr(B)
    errC = calErr(C)


    figure
    hold on;
    plot(x, errA, 'r-+', 'LineWidth',3);
    plot(x, errB, 'g-o', 'LineWidth',3);
    plot(x, errC, 'b-x', 'LineWidth',3);
    xlim([0,m])
    %ylim([-0.2,1.0])
    a = get(gca,'XTickLabel');
    set(gca,'XTickLabel',a,'FontName','Times','fontsize',20)
    b = get(gca,'YTickLabel');
    set(gca,'YTickLabel',b,'FontName','Times','fontsize',20)
    title(seqName, 'fontsize',20);
    legend('CV-CPU', 'CV-CUDA', 'NV-VPI');
    ylabel( 'tracked feature points distance error', 'FontSize', 18 );
    xlabel( 'tracked feature point index', 'FontSize', 18 );

end
end

function errA = calErr(A)
    %A: n x 8, [id, frame1.x, frame1.y, gt_frame2.x, gt_frame2.y, est_frame2.x, est_frame2.y, status]
    x0 = A(:,4); 
    y0 = A(:,5);

    dx = A(:, 6) - x0; 
    dy = A(:, 7) - y0;
    errA = sqrt(dx .* dx + dy .* dy);
    
    I = (A(:,8)==0); 
    errA(I)=nan;
end
