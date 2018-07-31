#-*- coding:utf-8 -*-
#07.urllib2_get.py

import urllib
import urllib2
from bs4 import BeautifulSoup

url ='http://query.sse.com.cn/infodisplay/queryBltnBookInfo.do?jsonCallBack=jsonpCallback55433&isPagination=true&isNew=1&bulletintype=L013&publishYear=2018&cmpCode=603611&startTime=&sortName=companyCode&direction=asc&pageHelp.pageSize=25&pageHelp.pageCount=50&pageHelp.pageNo=1&pageHelp.beginPage=1&pageHelp.cacheSize=1&pageHelp.endPage=5&_=1532675386426'

#url ='https://query.sse.com.cn/infodisplay/'

headers = { 
'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.11 (KHTML, like Gecko) Chrome/23.0.1271.64 Safari/537.11',
'Host':'query.sse.com.cn',
'Referer':'http://www.sse.com.cn/disclosure/listedinfo/periodic/',
'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
'Accept-Encoding':  'gzip, deflate',
'Accept-Language': 'zh-CN',
'Connection': 'keep-alive'
}
#values = {'inputCode':'000063'}
#pos_data = urllib.urlencode(values)
 
request = urllib2.Request(url,headers = headers)
page = urllib2.urlopen(request)
#page.encoding = 'utf-8'
soup = BeautifulSoup(page,"lxml")
html= soup.prettify()


print html
html=html.encode("utf-8")
with open('testi2.html','wb') as f:
     f.write(html)

#print(response.read())
