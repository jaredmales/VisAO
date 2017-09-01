pro get_ndoffsets

rp_5050 = get_focuspreset('0024') - get_focuspreset('0020')
ip_5050 = get_focuspreset('0034') - get_focuspreset('0030')
zp_5050 = get_focuspreset('0014') - get_focuspreset('0010')
ys_5050 = get_focuspreset('0044') - get_focuspreset('0040')

print, rp_5050, ip_5050, zp_5050, ys_5050

rp_win = get_focuspreset('1024') - get_focuspreset('1020')
ip_win = get_focuspreset('1034') - get_focuspreset('1030')
zp_win = get_focuspreset('1014') - get_focuspreset('1010')
ys_win = get_focuspreset('1044') - get_focuspreset('1040')

print, rp_win, ip_win, zp_win, ys_win

rp_800 = get_focuspreset('4024') - get_focuspreset('4020')
ip_800 = get_focuspreset('4034') - get_focuspreset('4030')
zp_800 = get_focuspreset('4014') - get_focuspreset('4010')
ys_800 = get_focuspreset('4044') - get_focuspreset('4040')

print, rp_800, ip_800, zp_800, ys_800

rp_950 = get_focuspreset('5024') - get_focuspreset('5020')
ip_950 = get_focuspreset('5034') - get_focuspreset('5030')
zp_950 = get_focuspreset('5014') - get_focuspreset('5010')
ys_950 = get_focuspreset('5044') - get_focuspreset('5040')

print, rp_950, ip_950, zp_950, ys_950

end


