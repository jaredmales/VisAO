pro visao_makefft, t, y, freq, amp, pwr, trange=trange

if(n_elements(trange) eq 2) then begin
   idx = where(t ge trange[0] and t le trange[1])
endif else begin
   idx = dindgen(n_elements(t))
endelse

dt = dblarr(n_elements(t)-1)

for i=0, n_elements(dt)-1 do dt[i] = t[i+1]-t[i]

freq = dindgen(n_elements(t[idx]))/double(n_elements(t[idx]))*(1./median(dt[idx]))

amp = fft(y[idx])

pwr = abs(amp)^2

end

