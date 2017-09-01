;+
; NAME: doron
; PURPOSE:
;  Measure readout noise from a set of CCD images, using the stddev of each pixel
;
; INPUTS:
;  gain ; the CCD gain
;
; KEYWORDS:
;  none
;
; OUTPUTS:
;  ron_e ; the readout noise, in electrons (ron_adu X gain)
;  ron_adu ; the readout noise, in adu
;
; EXAMPLE:
;  doron, 0.47, ron_e, ron_adu
;
; HISTORY:
;  Written 2011-04-20 (ish) by Jared Males, jrmales@email.arizona.edu
;  2012-05-26. Documentation updated by Jared Males
;
;-
pro doron_cube, ims, ron

; ;Make the image list
; spawn, 'ls *.fits > corrected.list'
; 
; ;Get the images from the list
; im_list, im
; 
; ;Cleanup
; spawn, 'rm corrected.list'


sz = (size(ims[*,*,0]))[1]

ron = dblarr(sz, sz)

for i=0,sz-1 do for j=0,sz-1 do ron[i,j] = stdev(ims[i,j,*])


end

