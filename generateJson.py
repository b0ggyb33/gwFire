def generate(n):
    example="{\n"
    example+="\t\"file\": \"images/Jumper_pos"
    example+= str(n)
    example+=".png\",\n"
    example+="\t\"name\": \"IMAGE_JUMP"
    example+= str(n)
    example+= "\",\n"
    example+="\t\"targetPlatforms\": [\n"
    example+="\t\t\"basalt\"\n"
    example+="\t],\n"
    example+="\t\"type\": \"bitmap\"\n"
    example+="},"
    return example

def generateBitmapCode(n):
	return "jumperBitmaps["+str(n)+"] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP"+str(n)+");"
	
if __name__=="__main__":
	for i in xrange(22):
		print generate(i)
	for i in xrange(22):
		print generateBitmapCode(i)
			