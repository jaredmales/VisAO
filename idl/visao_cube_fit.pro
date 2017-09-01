;+
; NAME: visa_cube_fit
;
; PURPOSE:
;  Fit a gaussian peak to a cube of images.
;
; INPUTS:
;  none
;
; KEYWORDS:
;  none
;
; OUTPUTS:
;    x      :   vector of x positions of the peak
;    y      :   y position of the peak
;   fw      :   vector of the fitted FWHMs (min)
;   pk      :   vector of the fitted peaks
;   mx      :   vector of the max values
;   mn      :   vector of mean values
;   foc     :   vector of focus stage positions
;   sig1    :   vector of fitted semi-major FWHMs
;   sig2    :   vector of fitted semi-minor FWHMs
;   ang     :   vector of fitted ellipse orientations
;
; EXAMPLE:
;  visao_cube_fit, ims, x, y, fw, pk, mx, mn, foc, sig1, sig2, ang
;
; HISTORY:
;  Written 2011-11-23 by Jared Males, jrmales@email.arizona.edu
;
;-
pro visao_cube_fit, ims, x, y, fw, pk, mx, mn, sig1, sig2, ang

dim1 = (size(ims))[1]
dim2 = (size(ims))[2]
nims = (size(ims))[3]


x = dblarr(nims)
y = x;
fw = x;
pk = x;
mx = x
mn = x
foc = x;
sig1 = x;
sig2 = x;
ang = x;


for h=0, nims-1 do begin
   status = strcompress(string(h+1) + '/' + string(nims), /rem)
   statusline, status, 0

   imraw=double(ims[*,*,h])

   mx[h] = max(imraw, i)
   mn[h] = mean(imraw)
   
   
   
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
         if (minx ge nsz) then minx = nsz-50
         maxx = idx[0]+50
         if (maxx lt 0) then maxx = 0
         if (maxx ge nsz) then maxx = nsz-1
         miny = idx[1]-50
         if (miny lt 0) then miny = 0
         if (miny ge nsz) then miny = nsz-50
         maxy = idx[1]+50
         if (maxy lt 0) then maxy = 0
         if (maxy ge nsz) then maxy = nsz-1
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

  r = MPFIT2DPEAK(imraw, A, /tilt, estimates = a0)
       
  x[h] = a[4]
  y[h] = a[5]
  fw[h] = min([a[2],a[3]])*2.*sqrt(2*alog(2))
  sig1[h] = max([a[2],a[3]])*2.*sqrt(2*alog(2))
  sig2[h] = min([a[2],a[3]])*2.*sqrt(2*alog(2))
  pk[h] = a[1]
  ang[h] = a[6]
   
endfor

statusline, /clear

end
