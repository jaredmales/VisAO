;+
; NAME: focusfit
; PURPOSE:
; 	Find best focus as defined by max counts, with a 2nd-order polynomial fit.
;
; INPUTS:
;	Two vectors containing (1) the focus positions and (2) the peak PSF counts
;
; KEYWORDS:
;	res = 1000.     ; inverse resolution - array length of findgen for where focus is max
;					; (optional) (default is 1000.)
;	/verbose        ; print messages
;	/plotit         ; plot it  -- including lines showing best focus position, and errors in fit
;
; OUTPUTS:
;	Best-fit focus position
;
; EXAMPLE:
;	print, focusfit(positions, counts, res=1000., /verbose, /plotit)
;
; HISTORY:
; 	Written 2012-02-15 by Katie Morzinski ktmorz@arizona.edu
;  2012-02-22 Added units to plot axes.  Jared Males, jrmales@email.arizona.edu
;
;-

FUNCTION focusfit, positions, counts, res=res, verbose=verbose, plotit=plotit
	myname = 'focusfit'

	if n_elements(res) eq 0 then res = 1000. ;size of array for focus positions = kinda the inverse resolution of the focus
	order = 2  ;second order polynomial
	
	N = n_elements(positions)

	x0 = positions     &     y0 = counts	
	coeff0 = poly_fit(x0,y0,order)
	y0fit = coeff0[0] + coeff0[1]*x0 + coeff0[2]*x0^2.
	dispersion = stddev(y0-y0fit)


	coeff_poly = poly_fit(positions,counts,order,$
			measure_error=fltarr(N)+dispersion,$
			sigma=sigma,status=status)
	if status gt 0 then begin
		print,'at '+myname+'.pro'
		print,'error:'
		print,'status = '+status
		print,'0 = Successful completion.'
		print,'1 = Singular array (which indicates that the inversion is invalid). Result is NaN.'
		print,'2 = Warning that a small pivot element was used and that significant accuracy was probably lost.'
		print,'3 = Undefined (NaN) error estimate was encountered.'
	endif

	pos_range = max(positions) - min(positions)
	pos_arr = ((findgen(res)+1.)/res)*pos_range + min(positions)
	counts_fit = coeff_poly[0] + coeff_poly[1]*pos_arr + coeff_poly[2]*pos_arr^2.
	peak_counts = max(counts_fit)
	best_focus = mean(pos_arr[where(counts_fit eq peak_counts)])

	; errors
	counts_fit_plus = (coeff_poly[0]+sigma[0]) + (coeff_poly[1]+sigma[1])*pos_arr + (coeff_poly[2]+sigma[2])*pos_arr^2.
	counts_fit_minus = (coeff_poly[0]-sigma[0]) + (coeff_poly[1]-sigma[1])*pos_arr + (coeff_poly[2]-sigma[2])*pos_arr^2.
	fit_dispersion = stddev(counts_fit_plus - counts_fit)
	
   pdx = uniq(positions)
   
   mns = dblarr(n_elements(pdx))
   stds = dblarr(n_elements(pdx))
   
   for i=0,n_elements(pdx) - 1 do begin
      idx = where(positions eq positions[pdx[i]])
      mns[i] = mean(counts[idx])
      if(n_elements(idx) gt 1) then begin
         stds[i] = stdev(counts[idx])/sqrt(n_elements(idx))
      endif else begin
         stds[i] = sqrt(mns[i])
      endelse
   endfor
   
   
	if keyword_set(plotit) then begin
		ploterror,positions[pdx],mns, stds, psym=2, symsize=1.3, title='Peak Counts', xtitle=textoidl('Focus Position (\mum)'),ytitle='Peak Counts (ADU)',charsize=1.8
		oplot,pos_arr,counts_fit,thick=2
		oplot,[best_focus,best_focus],[min(counts_fit),max(counts_fit)],linestyle=2,thick=2
		oplot,[min(pos_arr),max(pos_arr)],[peak_counts,peak_counts],linestyle=2,thick=2
		; plot errors
		;oplot,pos_arr,counts_fit_plus,thick=2,linestyle=1
		;oplot,pos_arr,counts_fit_minus,thick=2,linestyle=1
	endif;/plotit
	if keyword_set(verbose) then begin
		print,strcompress('#####  at '+myname+'.pro  #####')
		print,strcompress('measure_error = dispersion in counts = '+string(dispersion))
		print,strcompress('1-sigma error on poly_fit in counts = '+string(fit_dispersion))
		print,strcompress('peak counts at best focus = '+string(peak_counts))
		print,strcompress('position of best focus = '+string(best_focus))
		print,'#######################################'
	endif;/verbose
	

RETURN,best_focus
END
