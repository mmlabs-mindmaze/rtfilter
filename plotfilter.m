fin = fopen('filein.bin');
fout = fopen('fileout.bin');

nchann = fread(fin, 1, 'int32');

datin = fread(fin,[nchann,inf],'float32');
datout = fread(fout,[nchann,inf],'float32');

fclose(fin);
fclose(fout);

plot(datin(1,:),'r')
hold on
plot(datout(1,:),'b')
hold off
axis([0 600 -6 6])
nchann
