import zipfile
import re
import xml.dom.minidom
import io
from PIL import Image
from defusedxml.lxml import fromstring as _fromstring, tostring

z = zipfile.ZipFile('/home/huydang/Downloads/excelsample/fw-78.xlsx', 'r')
print(z.namelist())
src = z.read('xl/drawings/drawing2.xml')

# print(src)
# byte_src = io.BytesIO(src)
# img = Image.open(byte_src)
# img.save('/home/huydang/Downloads/excelsample/image1.png', 'png')
# exit()
uglyXml = xml.dom.minidom.parseString(src).toprettyxml(indent='  ')

text_re = re.compile('>\n\s+([^<>\s].*?)\n\s+</', re.DOTALL)
prettyXml = text_re.sub('>\g<1></', uglyXml)
with open('/home/huydang/Desktop/test_xml.xml', 'wb') as f:
    f.write(prettyXml.encode('utf-8'))
# print(z.namelist())


# class ImageExcel(object):
    
#     def __init__(self, path, file_name):
#         self.path = path
#         self.file_name = file_name
    
#     def read_image(self):
        
