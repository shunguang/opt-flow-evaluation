function x = readOneFrmFlow_gt( dir0, filename, goodStatusFlag)
%x, n x 8, [id, frame1.x, frame1.y, gt_frame2.x, gt_frame2.y, est_frame2.x, est_frame2.y, status]

f = [dir0, '/', filename];
x = readmatrix(f, "NumHeaderLines",1);


I = (x(:, 8)==goodStatusFlag);
x(:, 8) = double(I);
end

