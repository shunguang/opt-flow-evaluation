function x = readOneFrmFlow( fn, dir0, tag, goodStatusFlag, nExpectedRows )
%x, n x 4, [id, featurePt.x, featurePt.y, status(1-goodm 0-bad)]

s0 = [num2str(fn, '%05d'), '.txt'];
f = [dir0, '/', tag, s0];

x = nan(nExpectedRows, 4 );

y = readmatrix(f, "NumHeaderLines",1);
[m, four] =  size(y);
if m>nExpectedRows
    m = nExpectedRows;
end
for i = 1 : m
    x(i, :) = y(i, :);
    if y(i, 4) == goodStatusFlag
        x(i,4) = 1;   %good
    else
        x(i,4) = 0;  %bad
    end
end

end

