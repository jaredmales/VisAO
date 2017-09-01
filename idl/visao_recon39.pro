pro visao_recon39, dt, sv, tt1, tt2, subdir=subdir, recfile=recfile, recdir=recdir


if(n_elements(subdir) eq 0) then begin
   subdir = './'
endif

   
flist = strcompress(subdir + 'bcu39.list',/rem)

cmd = 'ls ' + strcompress(subdir + 'BCU*.fits',/rem) + ' > ' + flist

spawn, cmd


fnames=''
transread, unit, fnames, filename=flist, /debug, comment="#", FORMAT='(A200)'

fnames=strcompress(fnames, /rem)

visao_getimtypes, fnames, dateobs=dateobs, aorec=aorec, /usefnames


if(n_elements(recfile) eq 0) then begin
   recfile = aorec[0]
endif

if(n_elements(recdir) eq 0) then begin
   recdir = strcompress(getenv('VISAO_ROOT') + '/calib/visao/reconstructor/RecMats/', /rem)
   recf = strcompress(recdir + recfile, /rem)
endif else begin
   recf = recfile
endelse


nframes = n_elements(fnames)

outname = strcompress(subdir + 'reconout.txt',/rem)

cmd = 'Reconstruct' + ' ' + recf + ' ' + strcompress(subdir + 'bcu39.list',/rem) + ' ' + string(nframes) +  ' > ' + outname


spawn, cmd

readcol, outname, sv, tt1, tt2

dt = (dateobs-dateobs[0])*86400.

idx = where(tt1 lt 1e6 and tt1 gt -1e6 and tt2 lt 1e6 and tt2 gt -1e6 and sv gt 0. and sv lt 1e10)

dateobs = dateobs[idx]
sv = sv[idx]
tt1 = tt1[idx]
tt2 = tt2[idx]

end
