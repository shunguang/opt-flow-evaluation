function x = readOneFrmFlow_v2( fn, dir0, tag, goodStatusFlag, nExpectedRows )
%x, n x 6, [id, prevFrmFeaturePt.x, prevFrmFeaturePt.y, currFrmFeaturePt.x, currFrmFeaturePt.y, status]

s0 = num2str(fn, '%05d-');
f = [dir0, '/', s0, tag, '.txt'];

x = nan(nExpectedRows, 6 );

y = readmatrix(f, "NumHeaderLines",1);
[m, four] =  size(y);
if m>nExpectedRows
    m = nExpectedRows;
end
for i = 1 : m
    x(i, :) = y(i, :);
    if y(i, 6) == goodStatusFlag
        x(i,6) = 1;   %good
    else
        x(i,6) = 0;  %bad
    end
end

end

