function [nCnt, meanDist, stdDist] = calDist(A,B, outlierThd)
%A, n x 6, [id, prevFrmFeaturePt.x, prevFrmFeaturePt.y, currFrmFeaturePt.x, currFrmFeaturePt.y, status]
%B, n x 6, [id, prevFrmFeaturePt.x, prevFrmFeaturePt.y, currFrmFeaturePt.x, currFrmFeaturePt.y, status]

[mA,nA ]= size(A);
[mB,nB ]= size(B);
assert( mA==mB);
assert( nA==nB);

nCnt=0;
nOutliers=0;
z = nan(1,mA);
for k=1:mA
    if A(k, 6)==0 || B(k,6)==0
        continue;
    end

    nCnt = nCnt+1;
    dd = norm( A(k,4:5) -B(k,4:5) );
    if dd<outlierThd
        z(k) = dd;
    else
        nOutliers = nOutliers + 1;
    end
end

I = ~isnan(z);
y = z(I);
meanDist = mean(y);
stdDist = std(y);
end

