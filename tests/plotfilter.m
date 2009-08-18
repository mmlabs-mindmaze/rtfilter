fin = fopen('filein.bin');
fout = fopen('fileout.bin');

nchann = fread(fin, 1, 'int32');
nchann2 = fread(fout, 1, 'int32');

if nchann ~= nchann2
    error('Mismatch of nchann');
end

datin = fread(fin,[nchann,inf],'float32');
datout = fread(fout,[nchann,inf],'float32');
%datmatlab = filter(b,a,datin,[],2);

fclose(fin);
fclose(fout);

for ichann=1:6:nchann
    figure
    plot(datin(ichann,:),'r')
    hold on
    plot(datout(ichann,:),'b')
   % plot(datmatlab(ichann,:),'g--')
    hold off
    axis([0 600 -2 2])
end

nchann
