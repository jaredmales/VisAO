pro visao_linearity, etimes, adu, region=region, subdir=subdir

im = 1
visao_getimtypes, fnames, imtypes, exptime=exptime, ims=ims, subdir=subdir

visao_cube_darksub, ims, imtypes, EXPTIME=EXPTIME
 
etimes = exptime(uniq(exptime))
adu = dblarr(n_elements(etimes))

dim1 = (size(ims[*,*,0]))[1]
dim2 = (size(ims[*,*,0]))[2]

dsub = dblarr(dim1, dim2)

if(~keyword_set(region)) then region=[0, dim1-1, 0, dim2-1]

   
for i=0, n_elements(etimes)-1 do begin

   idx = where(exptime eq etimes[i])

;   print, 'Found ', n_elements(idx), ' images with exptime ', etimes[i]
   
   sdx = where( imtypes[idx] eq 0 )
   
   print, 'Found ', n_elements(sdx), ' science frames with exptime ',  etimes[i]
   
   for j =0, n_elements(sdx)-1 do begin
      dsub = dsub + ((ims[*,*,idx])[ *,*,sdx[j]])
   endfor
   
   dsub = dsub/double(n_elements(sdx))
   
   adu[i] = max(dsub[region[0]:region[1], region[2]:region[3]])
   
   dsub = dsub*0.
   
endfor

end

