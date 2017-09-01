pro get_focuspreset_key, pname, sname, prefix=prefix, subdir=subdir


if(n_elements(prefix) eq 0) then prefix='V47_'
if(n_elements(subdir) eq 0) then subdir='./'

srchstr = strcompress(subdir+prefix+'*.fits', /remove)
   
fnames = file_search(srchstr)
   
if(n_elements(fnames)) eq 0 then begin
   sname = 'error'
   pname = 'error'
   return
endif


imraw=mrdfits(fnames[0], 0, header, /silent)

   fw1pos = -1000
   for q=0, n_elements(header)-1 do begin
      hidx = strmatch(header,'VFW1POS *')
      if((where(hidx gt 0))[0] ne -1) then fw1pos = strn(strmid(header[where(hidx gt 0)], 10, 21))
   endfor
   
   fw2pos = -2000
   for q=0, n_elements(header)-1 do begin
      hidx = strmatch(header,'VFW2POS *')
      if((where(hidx gt 0))[0] ne -1) then fw2pos = strn(strmid(header[where(hidx gt 0)], 10, 21))
   endfor  

   fw3pos = -1000
   for q=0, n_elements(header)-1 do begin
      hidx = strmatch(header,'VFW3POS *')
      if((where(hidx gt 0))[0] ne -1) then fw3pos = strn(strmid(header[where(hidx gt 0)], 10, 21))
   endfor

   wollstr = '-1000'
   for q=0, n_elements(header)-1 do begin
      hidx = strmatch(header,'VWOLLSTN*')
      if((where(hidx gt 0))[0] ne -1) then wollstr = (strmid(header[where(hidx gt 0)], 10, 21))
   endfor

   fw1pos = long(fw1pos+0.5)
   fw2pos = long(fw2pos+0.5)
   fw3pos = long(fw3pos+0.5)
   woll = 1
   ;print, wollstr
   ;wollstr = 'IN       '
   if(strcmp(wollstr, "'O", 2) eq 1) then woll = 0
   ;print, fw1pos, ' ', woll, ' ', fw2pos, ' ', fw3pos

   sname = strcompress(string(fw1pos) + string(woll) + string(fw2pos) + string(fw3pos),/remove_all)
   
   pname = strcompress(getenv('VISAO_ROOT') + '/calib/visao/focus/presets/' + string(fw1pos) + string(woll) + string(fw2pos) + string(fw3pos) + '.preset',/remove_all)

   print, pname
end
