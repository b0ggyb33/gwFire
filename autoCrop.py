import Image
import numpy as np
import sys

im=np.asarray(Image.open(sys.argv[1]))

minx=miny=0
for i in xrange(im.shape[0]):
	if im[i].sum()>0:
		minx=i
	   	break


for i in xrange(im.shape[1]):
	if im[:,i].sum()>0:
		miny=i
	   	break
maxx,maxy=im.shape
for i in xrange(im.shape[0]-1,0,-1):
	if im[i].sum()>0:
		maxx=i
	   	break

for i in xrange(im.shape[1]-1,0,-1):
	if im[:,i].sum()>0:
		maxy=i
	   	break

im= im.astype('uint8') * 255
im = 255 - im 

print sys.argv[2]
print "offset: "+str(minx)+", "+str(miny)
print "size: "+str(maxx+1-minx)+", "+str(maxy+1-miny)
print "{"+str(minx)+","+str(miny)+","+str(maxx)+","+str(maxy)+"},"

outIm=Image.fromarray(im[minx:maxx+1,miny:maxy+1], 'L')
outIm.save(sys.argv[2])