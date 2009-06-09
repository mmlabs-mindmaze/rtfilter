function ccoeffilt(n,fc)

ripple = 0.005;
wn = fc;
R = 10*log10(1+ripple);
%[num, denum] = butter(n,wn);
[num, denum] = cheby1(n,R,wn);

denum = -denum(2:end);

numlen = length(num);
denumlen = length(denum);

fprintf('// fc = %.3f (wn=%.3f in MATLAB)\n',fc,wn);
fprintf('float num[%u] = {',numlen);
fprintf('%15.15e, ',num);
fprintf('};\n');
fprintf('float denum[%u] = {',denumlen);
fprintf('%15.15e, ',denum);
fprintf('};\n');

