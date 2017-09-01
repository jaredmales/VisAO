;+
; NAME: visao_focuscurve
; 
; DESCRIPTION:
;   Reduces and analyzes a focus curve data set from the VisAO camera.  Optionally writes the result to the calib files on VisAO.
;   By default looks in current directory for files, but this can be changed with the subdir keyword.
;
; INPUTS:
;  none
;
; KEYWORDS:
;  region   :   set to a 4 element vector, [x0,x1,y0,y1] defining the region of the images to analyze. passed to
;  subdir   :   set to a string, will analyze images in the specified directory
;  preset   :   set to write the fitted focus position to a preset file
;  calib    :   set to write the fitted focus position to the cal file
;  interact :   ask whether to write the cal or preset
;
; OUTPUT:
;   fw      :   vector of the fitted FWHMs
;   pk      :   vector of the fitted peaks
;   mx      :   vector of the max values
;   mn      :   vector of mean values
;   vfocpos :   vector of focus stage positions
;   sig1    :   vector of fitted semi-major FWHMs
;   sig2    :   vector of fitted semi-minor FWHMs
;   ang     :   vector of fitted ellipse orientations
;   fnames  :   vecor of file names
;
; EXAMPLE:
;  visao_focuscurve, region=[256,768,256,768], /cal
;
; MODIFICATION HISTORY:
;  Written 2010/03/01 by Jared Males (jrmales@email.arizona.edu)
;  2012/11/13 Added iteraction and updated docs.
;
; BUGS/WISH LIST:
;   None.  This is perfect code.
;
;-

pro visao_focuscurve, fw, pk, mx, mn, vfocpos, sig1, sig2, ang, fnames, region=region, subdir = subdir, preset=preset, calib=calib, interact=interact


;visao_basic_darksub, dsub, foc, subdir=subdir

visao_getimtypes, fnames, imtypes, vfocpos=vfocpos, ims=ims, subdir=subdir, region=region

visao_cube_darksub, ims, imtypes, vfocpos=vfocpos

;foc = vfocpos

visao_cube_fit, ims, x, y, fw, pk, mx, mn,  sig1, sig2, ang


minsig = 1.5
maxsig = 30.

device, retain=2

idx = where(sig1 lt maxsig and sig1 gt minsig and sig2 lt maxsig and sig2 gt minsig)

nrejected = n_elements(sig1) - n_elements(idx)

print, "rejecting: ", nrejected
sig1 = sig1[idx]
sig2 = sig2[idx]

ellip = (sig1-sig2)/sig1

vfocpos = vfocpos[idx]
pk =pk[idx]

fw = 0.5*(sig1+sig2)



ufoc = uniq(vfocpos)
umean = dblarr(n_elements(ufoc))
uellip = umean

for i=0, n_elements(ufoc)-1 do begin
   jdx = where(vfocpos eq vfocpos[ufoc[i]])

   if(n_elements(jdx) gt 1) then begin
      umean[i] = mean(fw[jdx])
      uellip[i] = mean(ellip[jdx])
   endif else begin
      umean[i] = (fw[jdx])
      uellip[i] = (ellip[jdx])
   endelse
endfor

window, 0
fitfoc = focusfit(vfocpos, pk, /plotit, /verbose)

window, 1

plot, vfocpos, sig1, psym=2, title = 'FWHM',xtitle=textoidl('Focus Position (\mum)'), ytitle = 'FWHM (pixels)', yrange=[.9*min([sig1,sig2]), 1.1*max([sig1,sig2])], /ystyle,charsize=1.8
oplot, vfocpos, sig2, psym=1


fdx = sort(vfocpos[ufoc])

oplot, (vfocpos[ufoc])[fdx], umean[fdx]
oplot,[fitfoc,fitfoc],[0,1000],linestyle=2,thick=2

window, 2

plot, vfocpos, (sig1-sig2)/sig1, psym=2, title = 'Ellipticity',xtitle=textoidl('Focus Position (\mum)'), ytitle='Ellipticity', charsize=1.8
oplot,[fitfoc,fitfoc],[0,1],linestyle=2,thick=2





minfw = min(fw, i)
minfoc = (vfocpos)[i]


minufw = min(umean, i)
minufoc = (vfocpos[ufoc])[i]
focellip = uellip[i]

mins1 = min(sig1, i)
mins1foc = vfocpos[i]

mins2 = min(sig2, i)
mins2foc = vfocpos[i]

maxpk = max(pk, mpi)
focpk = vfocpos[mpi]

if(keyword_set(preset) or keyword_set(calib) or keyword_set(interact)) then begin
;Get the current focus calibration position
fcal = get_focuscal()

endif

;Get the preset filename and key
get_focuspreset_key, pname, sname, subdir=subdir





print, ' '
print, ' '
print, '***************************************************'
print, '             VisAO Focus Analysis Complete'
print, ' '
print, 'Parabolic best fit        ', fitfoc
print, ' '
print, 'preset: ', pname
print, ' '
print, '***************************************************'
print, 'Max peak/position         ', maxpk, focpk
print, 'Min. mean fwhm/position   ',minufw, minufoc
print, 'Mean ellipticity at focus ', focellip
print, '***************************************************'
print, 'Minimum fwhm/position ', minfw, minfoc
print, 'Minimum semi-maj/pos  ', mins1, mins1foc
print, 'Minimum semi-min/pos  ', mins2, mins1foc
print, '***************************************************'

wcal = 0
wpre = 0

if(keyword_set(interact)) then begin
   ok_str = 'ok'

   if(sname eq 'XXXX') then begin ; used to be 0014, but we don't do that anymore
      
      read, ok_str, prompt='Write calibration? [y/n] (n) '
      if(ok_str eq 'y' or ok_str eq 'Y' ) then wcal = 1
           
   endif else begin
   
      read, ok_str, prompt='Write preset? [y/n] (n) '
      if(ok_str eq 'y' or ok_str eq 'y') then wpre = 1
      
   endelse
endif

if(keyword_set(calib) or wcal eq 1) then begin
   fcalwrit = 'YES'
   write_focuscal, fitfoc
endif else begin
   fcalwrit = 'NO'
endelse   

if(keyword_set(preset) or wpre eq 1) then begin
   forprint, textout=pname, fitfoc - fcal, /NOCOMMENT, /silent
endif else begin
   pname = 'NO'
endelse
print, ' '
print, 'cal written: ', fcalwrit
print, 'preset written: ', pname
print, ' '

end

