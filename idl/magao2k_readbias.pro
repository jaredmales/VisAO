function magao2k_readbias, dir,prefix

   ims = readrawcube39(dir, prefix)

   bias0 = median(ims[0:39,0:39,*])
   max0 = max(ims[0:39,0:39,*])
   min0 = min(ims[0:39,0:39,*])
   
   bias1 = median(ims[40:*,0:39,*])
   max1 = max(ims[40:*,0:39, *])
   min1 = min(ims[40:*,0:39, *])
   
   bias2 = median(ims[0:39,40:*,*])
   max2 = max(ims[0:39,40:*,*])
   min2 = min(ims[0:39,40:*,*])
   
   bias3 = median(ims[40:*,40:*,*])
   max3 = max(ims[40:*,40:*,*])
   min3 = min(ims[40:*,40:*,*])
   
   return, [bias0, max0, min0, bias1, max1, min1, bias2, max2, min2, bias3, max3, min3]
end



