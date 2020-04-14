from urllib import request
import re

#按F12查看
headers={
    'User-Agent':
    'Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36 SE 2.X MetaSr 1.0'}


#获取网页源代码函数
def gethtml(url):
    url_request = request.Request(url, headers=headers)  # 加上头部
    response = request.urlopen(url_request)
    data = response.read().decode('gb2312','ignore')#解决中文乱码问题
    #print(data)  # 输出源代码
    return data

#查询函数,返回一个列表
def search(msg,html):
    list=re.findall(msg,html)#正则表达式查询所有符合条件的msg，保存在列表
    return list

#查询网页标题
def search_name(html):
    begin=html.find('title')
    end = html.find('</title')
    name = html[begin + 6:end]
    return name

#获取磁力链接
def get_ftp(html):
    begin = html.find('magnet')
    end = html.find('"><strong>')
    ftp = html[begin:end]
    return ftp

#主页
url='https://www.dytt8.net/html/gndy/dyzz/index.html'
html=gethtml(url)
#print(html)

purl='<a href="/html/gndy/dyzz/.+?.html'
plist=search(purl,html) #查找符合条件的网页url

n=0
for each in plist:
    pos=each.find('/html')
    each='https://www.dytt8.net'+each[pos:]
    html=gethtml(each)
    name=search_name(html)
    print(name)
    #print(each)
    ftp=get_ftp(html)
    print(ftp)
    n += 1
    #写入文件

    filename='xunlei.txt'
    #print(filename)
    f=open(filename,'a',encoding='utf-8')
    f.write(name)
    f.write('\n')
    f.write(ftp)
    f.write('\n\n')
print('total='+str(n))

