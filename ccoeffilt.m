function [b,a] = ccoeffilt(n,fc)

ripple = 0.005;
wn = fc;
R = 10*log10(1+ripple);
%[num, denum] = butter(n,wn);
[num, denum] = cheby1(n,R,wn);

numlen = length(num);
denumlen = length(denum);

fprintf('// fc = %.3f (wn=%.3f in MATLAB or ccoeffilt(%u,%f))\n',fc,wn,n,fc);
fprintf('float num[%u] = {',numlen);
fprintf('%15.15e, ',num);
fprintf('};\n');
fprintf('float denum[%u] = {',denumlen);
fprintf('%15.15e, ',denum);
fprintf('};\n');
b = num;
a = denum;


