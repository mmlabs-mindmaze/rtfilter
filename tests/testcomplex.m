

%   READ FROM FILES

file=fopen('data.bin','r');
INPUT=fread(file,inf,'single');
fclose(file);
file=fopen('Filt.bin','r');
FILT=fread(file,inf,'single');
fclose(file);
file=fopen('features.bin','r');
FEAT=fread(file,inf,'single');
fclose(file);

clear file;
clear ans;

num_poles=FEAT(1);
fl=FEAT(2);
fh=FEAT(3);
fs=FEAT(4);
nsample=FEAT(5);

t=(0:1/fs:(nsample-1)/fs);


% INPUT SIGNAL % 2 channels
in1=(INPUT(1:2:length(INPUT)));
in2=(INPUT(2:2:length(INPUT)));

%  COMPLEX FILTER MATALAB CODE
[A,B]=compute_complex_filter(num_poles,fl,fh,fs);

z1=filter(single(B),single(A),single(in1));
z2=filter(single(B),single(A),single(in2));

% FILTERED SIGNAL FROM C CODE % 2 channels
filt1=(FILT(1:4:length(FILT)) + 1i*FILT(2:4:length(FILT)));
filt2=(FILT(3:4:length(FILT)) + 1i*FILT(4:4:length(FILT)));




figure(1);
plot(t,in1); grid on;
title('INPUT SIGNAL (1)');

figure(2);
plot(t,in2); grid on;
title('INPUT SIGNAL (2)');

figure(3);
plot(t,real(z1),'b'); hold on
plot(t,abs(z1),':');
plot(t,imag(z1),'r'), hold off
title('COMPLEX BANDPASS FILTERED SIGNAL (1) MATLAB CODE');

figure(4);
plot(t,real(filt1),'b'); hold on
plot(t,abs(filt1),':');
plot(t,imag(filt1),'r'), hold off
title('COMPLEX BANDPASS FILTERED SIGNAL (1) C CODE');

figure(5);
plot(t,real(z2),'b'); hold on
plot(t,abs(z2),':');
plot(t,imag(z2),'r'), hold off
title('COMPLEX BANDPASS FILTERED SIGNAL (2) MATLAB CODE');

figure(6);
plot(t,real(filt2),'b'); hold on
plot(t,abs(filt2),':');
plot(t,imag(filt2),'r'), hold off
title('COMPLEX BANDPASS FILTERED SIGNAL (2) C CODE');










