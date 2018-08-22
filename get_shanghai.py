#-*- coding:utf-8 -*-
#07.urllib2_get.py

import urllib
import urllib2
import re
from bs4 import BeautifulSoup
import numpy as np
import pandas as pd
import datetime
import time
#url ='http://query.sse.com.cn/infodisplay/queryBltnBookInfo.do?jsonCallBack=jsonpCallback55433&isPagination=true&isNew=1&bulletintype=L013&publishYear=2018&cmpCode=603611&startTime=&sortName=companyCode&direction=asc&pageHelp.pageSize=25&pageHelp.pageCount=50&pageHelp.pageNo=1&pageHelp.beginPage=1&pageHelp.cacheSize=1&pageHelp.endPage=5&_=1532675386426'

i = datetime.datetime.now()
#print ("当前的日期是%s" %i)
time_string = "%s-%s-%s"%(i.year,i.month,i.day)
print time_string 
url ='http://query.sse.com.cn/infodisplay/queryBltnBookInfo.do?jsonCallBack=jsonpCallback55433&isNew=1&publishYear=2018'
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
def compare_time(time1,time2):
    s_time = time.mktime(time.strptime(time1,'%Y-%m-%d'))
    e_time = time.mktime(time.strptime(time2,'%Y-%m-%d'))
    return int(s_time) - int(e_time)
    
def my_save(filename,contents):
    fh=open(filename,'w')
    fh.write(contents)
    fh.close()
 
request = urllib2.Request(url,headers = headers)
page = urllib2.urlopen(request)
#page.encoding = 'utf-8'
soup = BeautifulSoup(page,"lxml")
html = soup.select('p')
string1 = str(html[0])
string2 = string1.split('ROWNUM_')
df=pd.DataFrame(columns=['Name','code','type','publishDate0','actualDate'])
for string in string2:
    name= re.findall(r'companyAbbr":"(.+?)","',string)
    code= re.findall(r'companyCode":"(.+?)","',string)
    report_type= re.findall(r'bulletinType":"(.+?)","',string)
    date = re.findall(r'publishDate0":"(.+?)","',string)
    
    actual =  re.findall(r'actualDate":"(.+?)","',string)
    if len(actual) == 0 and len(date)!=0 and compare_time(str(date[0]),time_string) > 0:
    	df=df.append(pd.DataFrame({'Name':name,'code':code,'type':report_type,'publishDate0':date}),ignore_index=True)
    #else:
    #    df=df.append(pd.DataFrame({'Name':name,'code':code,'type':report_type,'publishDate0':date,'actualDate':actual}),ignore_index=True)
    #result = 'name:'+ str(name) +'code :'+str(code) + 'type :'+ str(report_type) + 'date :' + str(date)
    #print result
df.sort_values(by = "publishDate0",ascending = True,inplace = True)
df.to_excel('ready_to_report.xlsx')


#print(response.read())
