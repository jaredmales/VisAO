pro magao2k_reduce_response_grid, dir, clamps, samples, reps, rongrid, gaingrid


gaingrid = fltarr(n_elements(clamps), n_elements(samples))
rongrid = gaingrid

for i=0,n_elements(clamps)-1 do begin
   for j=0,n_elements(samples)-1 do begin

      griddir = strcompress( dir + '/clamp_' + string(clamps[i]) + '_sample_' + string(samples[j]), /rem)
 
      magao2k_reduce_response_curve, griddir, reps, dark, ronadu, stds, flats, gainmap, gain

      gaingrid[i,j] = gain
      
      rongrid[i,j] = ronadu;*gain
   endfor
endfor

end
