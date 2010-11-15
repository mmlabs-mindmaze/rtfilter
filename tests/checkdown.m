function [datin, datout, datmatlab] = checkdown(filein, fileout)

fout = fopen(fileout);

datouttype = fread(fout, 1, 'uint32');
nchannout = fread(fout, 1, 'uint32');
r = fread(fout, 1, 'uint32');
datout = fread_array(fout, [nchannout, inf], datouttype);

[b,a] = cheby1(8, 0.05, 0.8/r);

fclose(fout);

fin = fopen(filein);

datintype = fread(fin, 1, 'uint32');
nchannin = fread(fin, 1, 'uint32');
datin = fread_array(fin, [nchannin, inf], datintype);

fclose(fin);


if (nchannin ~= nchannout)
    error('data params differs in the 2 files');
end

%datmatlab = zeros(size(datout));
%for i=1:size(datin, 1)
%	datmatlab(i,:) = decimate(double(datin(i,:)),r);
%end
tmpbuff = filter(b,a,datin,[],2);
datmatlab = tmpbuff(:, r:r:end);

diffval = abs(datmatlab - datout);
errvals = max(diffval,[],2) ./ max(abs(datmatlab),[],2);
errval = max(errvals);
fprintf('Error value = %10.10g\n',errval);
return;

function A = fread_array(fid, arrsize, typeval)
    type = '*float32';
    if bitand(typeval,1)
        type = '*float64';
    end
    
    c = 0;
    if bitand(typeval,2)
        c = 1;
        arrsize(1) = arrsize(1)*2;
    end
    
    A = fread(fid, arrsize, type);
    
    if c == 1
        A = A(1:2:end,:) + 1i*A(2:2:end,:);
    end
return
