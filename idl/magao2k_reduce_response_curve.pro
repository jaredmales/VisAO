pro magao2k_reduce_response_curve, dir, reps, darks, ronadu, stds, flats, gainmap, gain, mask, ron


darkims = readrawcube39(dir, 'darksN')

;dark = median(darks, dim=3)

ron = darkims[*,*,0]
for i=0,79 do begin
   for j=0,79 do begin
      ron[i,j] = stddev(darkims[i,j,*])
   endfor
endfor

if(n_elements(reps) eq 0) then begin
   ronadu = median(ron[20:60,20:60])
   darks = median(darkims, dim=3)
   gain = 0
   return
endif
   
flats = fltarr(80,80, n_elements(reps))
darks = fltarr(80,80, n_elements(reps)+1)
stds = fltarr(80,80, n_elements(reps))
darks[*,*,0] = median(darkims, dim=3)

for i=0,n_elements(reps)-1 do begin
   darkims = readrawcube39(dir, strcompress('darks_reps'+ string(reps[i]), /rem))

   darks[*,*,i+1] = median(darkims, dim=3)

   rawflats = readrawcube39(dir, strcompress('reps' + string(reps[i]), /rem))

;   get_cubedims, rawflats, dim1, dim2, nims
   dim1 = 80
   dim2 = 80
   nims = 1000

   for j=0,nims-1 do begin
      rawflats[*,*,j] = rawflats[*,*,j] - darks[*,*,i+1]
   endfor

   flats[*,*,i] = median(rawflats, dim=3)

   for j=0,dim1-1 do begin
      for k=0, dim2-1 do begin

         stds[j,k,i] = stddev(rawflats[j,k,*])
      endfor
   endfor

endfor

gainmap = fltarr(80,80)

vc = stds^2
for i=0,80-1 do for j=0,80-1 do gainmap[i,j] = 1./(linfit(flats[i,j,*], vc[i,j,*]))[1]

idx = where(flats[*,*,n_elements(reps)-1] gt 0.75*median(flats[*,*,n_elements(reps)-1]))
mask = fltarr(80,80)
mask[idx] = 1

idx = where(mask gt 0 and gainmap gt 0 and gainmap lt 2)
;gain = median(gainmap[idx])

hist = histogram(gainmap[idx], bins=.05, loc=sx, min=0, max=2)
mx = max(hist, i)
gain = sx[i] + 0.5*0.05

ronadu = median(ron[idx])
end
