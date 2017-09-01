function readrawcube39, dir, prefix

if n_elements(prefix) eq 0 then prefix = ''
patt = prefix + '*.raw'
fnames = file_search(dir, patt)

ims = intarr(80, 80, n_elements(fnames))

for i=0,n_elements(fnames)-1 do begin
   openr, lun, fnames[i], /get_lun

   im =read_binary(lun, data_type=2, data_dims=6400)
   ims[*,*,i] = reform(im, 80, 80)
   close, lun
   free_lun, lun
endfor

return, ims

end


