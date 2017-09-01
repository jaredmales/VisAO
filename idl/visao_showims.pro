pro visao_showims, ims, n, pt

for i=0, n-1 do begin
   tvscl, ims[*,*,i]
   wait, pt
endfor

end