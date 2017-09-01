
;+
; NAME: peakfit_list
;
; PURPOSE:
;  Fit a gaussian peak to a set of VisAO images, with dark subtraction.
;
; INPUTS:
;  none
;
; KEYWORDS:
;  region   :   set to a 4 element vector, [x0,x1,y0,y1] defining the region of the images to analyze. passed to peakfit_list
;
; OUTPUTS:
;    n      :   image number
;    x      :   x position of the peak
;    y      :   y position of the peak
;   fw      :   vector of the fitted FWHMs (min)
;   pk      :   vector of the fitted peaks
;   mx      :   vector of the max values
;   mn      :   vector of mean values
;   foc     :   vector of focus stage positions
;   sig1    :   vector of fitted semi-major FWHMs
;   sig2    :   vector of fitted semi-minor FWHMs
;   ang     :   vector of fitted ellipse orientations
;   fnames  :   vecor of file names
;
; EXAMPLE:
;  peakfit_list, n,x, y, fw, pk, mx, mn, foc, sig1, sig2, ang, fnames, region=[256,768,256,768]
;
; HISTORY:
;  Written 2011-03-01 (ish) by Jared Males, jrmales@email.arizona.edu
;  2012-11-13. Documentation updated by Jared Males
;
;-
pro peakfit_list, x, y, fw, pk, mx, mn, foc, sig1, sig2, ang, fnames, region=region

dark_list = strcompress('./darks.list', /remove_all)


corrected_file=''
transread, unit, corrected_file, filename=dark_list, /debug, $
  comment="#", FORMAT='(A200)'

corrected_dir = strarr(n_elements(corrected_file))

rgx = stregex(corrected_file, "((.*/)*)(.*)", /subexpr)

for q=0, n_elements(corrected_file)-1 do begin
   corrected_dir(q) = strcompress(strmid(corrected_file(q), 0, rgx(4*q+3)), /remove_all)
   corrected_file(q)=strcompress(strmid(corrected_file(q), rgx(4*q+3)), /remove_all)
endfor

imraw=mrdfits(strcompress(corrected_dir[0] + corrected_file[0], /remove_all), 0, /silent)

for h=1, n_elements(corrected_file)-1 do begin

   ;print, h+1, '/', n_elements(corrected_file)
   
   imraw=imraw+mrdfits(strcompress(corrected_dir[h] + corrected_file[h], /remove_all), 0,  /silent)
   ;print,header
endfor

imraw = double(imraw) / double(n_elements(corrected_file))

imdark = imraw

corrected_list=strcompress('./corrected.list', /remove_all)

corrected_file=''
transread, unit, corrected_file, filename=corrected_list, /debug, $
  comment="#", FORMAT='(A200)'

corrected_dir = strarr(n_elements(corrected_file))

rgx = stregex(corrected_file, "((.*/)*)(.*)", /subexpr)

for q=0, n_elements(corrected_file)-1 do begin
   corrected_dir(q) = strcompress(strmid(corrected_file(q), 0, rgx(4*q+3)), /remove_all)
   corrected_file(q)=strcompress(strmid(corrected_file(q), rgx(4*q+3)), /remove_all)
endfor

x = dblarr(n_elements(corrected_file))
y = x;
fw = x;
pk = x;
mx = x
mn = x
foc = x;
sig1 = x;
sig2 = x;
ang = x;
fnames = corrected_file


for h=0, n_elements(corrected_file)-1 do begin
   status = strcompress(string(h+1) + '/' + string(n_elements(corrected_file)), /rem)
   statusline, status, 0
    
   imraw=double(mrdfits(strcompress(corrected_dir[h] + corrected_file[h], /remove_all), 0, header, /silent)) - imdark

   

   ;print,header
   
  if keyword_set(region) then begin
      imraw = imraw[region[0]:region[1],region[2]:region[3]]
      print,h
      tvscl, imraw
 endif

   nsz = (size(imraw))[1]

   
   ;imraw = imraw[0:31,0:15]

   mx[h] = max(imraw, i)
   mn[h] = mean(imraw)




   ;if(max(imraw)-mean(imraw) gt 100. and median(imraw) lt 800.) then begin

      idx = array_indices(imraw,i)
      if(nsz gt 255) then begin
         
         minx = idx[0]-50
         if (minx lt 0) then minx = 0
         if (minx gt nsz) then minx = nsz-50
         maxx = idx[0]+50
         if (maxx lt 0) then maxx = 0
         if (maxx gt nsz) then maxx = nsz-1
         miny = idx[1]-50
         if (miny lt 0) then miny = 0
         if (miny gt nsz) then miny = nsz-50
         maxy = idx[1]+50
         if (maxy lt 0) then maxy = 0
         if (maxy gt nsz) then maxy = nsz-1
         ;print, minx, maxx, miny, maxy
         imraw = imraw[minx:maxx,miny:maxy]
         A0 = [median(imraw), mx[h] - median(imraw), 2., 2., .5*(maxx-minx), .5*(maxy-miny), 0.]
      endif else begin
	      minx = 0
	      maxx = 63
	      miny  = 0
	      maxy = 63


         A0 = [median(imraw), mx[h] - median(imraw), 2., 2., idx[0], idx[1], 0.]
      endelse
      ;imraw = imraw[idx[0]-20:idx[0]+20,idx[1]-20:idx[1]+20]
      ;imraw = imraw[0:31,0:15]

   
      r = MPFIT2DPEAK(imraw, A, /tilt, estimates = a0)
      ;r = gauss2dfit(imraw, A)

      for q=0, n_elements(header)-1 do begin
         hidx = strmatch(header,'VFOCPOS*')
         if((where(hidx gt 0))[0] ne -1) then foc[h] = strn(strmid(header[where(hidx gt 0)], 10, 21))
      endfor
       
      x[h] = a[4]+minx
      y[h] = a[5]+miny
      fw[h] = min([a[2],a[3]])*2.*sqrt(2*alog(2))
      sig1[h] = max([a[2],a[3]])*2.*sqrt(2*alog(2))
      sig2[h] = min([a[2],a[3]])*2.*sqrt(2*alog(2))
      pk[h] = a[1]
      ;pk[h] = focusphot(imraw, /fw1)
      ang[h] = a[6]
   ;endif
   
endfor

statusline, /clear

end
