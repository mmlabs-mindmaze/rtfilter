


 function [A,B]=compute_complex_filter(n,fl,fh,fs)


R=0.5;                                                                                                                    ;
alpha=(fl+fh)/2;
fc=(fh-fl)/2; 


Wl=2*fc/fs;
Wx=2*pi*alpha/fs;

 
% REAL LOW PASS FILTER
 [b,a] = cheby1(n,R,Wl,'low'); 

% CALCULATE FILTER DELAY
a1=fliplr(a);
c=conv(b,a1);
cr = c.*[0:(length(c)-1)]; 


N=length(c);

for n=1:N
    Y(n)=0;
    X(n)=0;
    for k=1:N
        Y(n)= Y(n) + cr(k)*exp(-2*pi*1i)*(k-1)*(n-1)/N;
        X(n)= X(n) + c(k)*exp(-2*pi*1i)*(k-1)*(n-1)/N;
    end
end


delay=max(real(Y/X));


% COMPLEX FILTER COEFFICIENT
 
 for k=1:length(a) 
     A(k)=a(k)*exp(1i*Wx*k);
 end
 
 for k=1:length(b) 
     B(k)=2*exp(-1i*Wx*delay)*b(k)*exp(1i*Wx*k);
 end
 
 end






