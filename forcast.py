# -*- coding:utf-8 -*
import tushare as ts
import numpy as np
import pandas as pd
import datetime
import chardet
import urllib
import urllib2
import re
from bs4 import BeautifulSoup
import time
from pandas import  Series,DataFrame

def get_relation(stock1,stock2):
        hist_data = ts.get_hist_data(stock1,start='2018-05-01')
        if hist_data is None:
                     return 0
        hist_data.sort_values(by = "date",ascending = True,inplace = True)
        hist_data_second = ts.get_hist_data(stock2,start='2018-05-01')
        if hist_data_second is None:
                     return 0
        hist_data_second.sort_values(by = "date",ascending = True,inplace = True)
        result = pd.concat([hist_data,hist_data_second],axis = 1)
        result = result['close']
        result = result.dropna(how = 'any')
        #result.to_excel('result.xlsx')
        corr_result= result.corr()
        result=np.array(corr_result.iloc[1:3,0:1])
        return result[0][0]

year = datetime.datetime.now().strftime('%Y')
month =  datetime.datetime.now().strftime('%m')
day =  datetime.datetime.now().strftime('%d')
second = datetime.datetime.now().strftime('%s')
season = int(month) /3 +1
basic = ts.get_stock_basics()
basic.to_excel( year+month+day+second + '_basics.xlsx')

grouped_pe = basic['pe'].groupby(basic['industry'])

grouped_pe.mean().to_excel( year+month+day+second + '_grouped_pe.xlsx')

grouped_pb = basic['pb'].groupby(basic['industry'])
#print grouped.mean()
grouped_pb.mean().to_excel( year+month+day+second + '_grouped_pb.xlsx')

#np_industry = np.array(grouped_pb.mean().index)
grouped_industry=pd.concat([grouped_pe.mean(),grouped_pb.mean()],axis =1 ,join = 'inner')
grouped_industry.to_excel( year+month+day+second + '_grouped_industry.xlsx')
np_industry = np.array(grouped_pb.mean().index)
#for industry in np_industry:
#    current_industy = basic[basic['industry'].isin([str(industry)])]
#    current_industy.to_excel(str(industry)+ '.xlsx')

yj_current_season=ts.forecast_data(int(year),season)
yj_last_season=ts.forecast_data(int(year),season-1)

yj_last_season_index=yj_last_season.set_index('code')
yj_curren_seaon_index=yj_current_season.set_index('code')
yj_index=pd.concat([yj_curren_seaon_index,yj_last_season_index],axis =0 ,join = 'outer')
#yj_index.to_excel('index_yeji.xlsx')
result = pd.concat([yj_index,basic],axis =1 ,join = 'inner')
#result_select = result[result['type'].isin([u'\u9884\u5347',u'\u9884\u589e'])]
result_select = result[result['type'].isin([u'\u9884\u589e'])]
result_select.sort_values(by = "report_date",ascending = False,inplace = True)
result_select = result_select[result_select['report_date'].isin([np.array(result_select['report_date'])[0]])]

for code in np.array(result_select.index):
	result_select.ix[str(code),'mean-pe'] = grouped_pe.mean()[result_select.ix[str(code),'industry']] 
	hist_data = ts.get_hist_data(str(code),start='2018-05-01')
	if hist_data is not None:
           		hist_data.sort_values(by = "date",ascending = False,inplace = True)
           		hist_data = hist_data.iloc[0:5,:]
           		#five_day_everage = hist_data['close'].mean()
           		#hist_data.to_excel( year+month+day+second+str(code) + 'history.xlsx')
			result_select.ix[str(code),'five-day-mean'] = hist_data['close'].mean()
                        close_price =  np.array(hist_data['close'])
                        if close_price.size > 0:
        			result_select.ix[str(code),'last_day_price'] = np.array(hist_data['close'])[0]
                                result_select.ix[str(code),'increase-rate'] =  \
                                 (np.array(hist_data['close'])[0] - hist_data['close'].mean())/hist_data['close'].mean()
                    
                                result_select.ix[str(code),'touzhijiazhi'] = \
                              (result_select.ix[str(code),'totalAssets']*10000)/(result_select.ix[str(code),'totals']*10000*10000)  

                                result_select.ix[str(code),'price-values'] = \
                             result_select.ix[str(code),'touzhijiazhi'] /result_select.ix[str(code),'last_day_price']
                                if result_select.ix[str(code),'pe'] == 0:
                                   result_select.ix[str(code),'pe'] = result_select.ix[str(code),'mean-pe']
                                result_select.ix[str(code),'pray-values'] = \
                             result_select.ix[str(code),'price-values'] * result_select.ix[str(code),'npr']/100.0 \
                             *result_select.ix[str(code),'mean-pe'] /result_select.ix[str(code),'pe'] \
                             *hist_data['close'].mean()/result_select.ix[str(code),'last_day_price']

 
result_select.to_excel( year+month+day+second + '_yeji.xlsx')

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
df.sort_values(by = "publishDate0",ascending = True,inplace = True)
#df= df.iloc[0:16,:]
df.to_excel('ready_to_report.xlsx')


np_ready_report = np.unique(np.array(df['code']))


np_increase_report = np.array(result_select.index)
forcast=pd.DataFrame()
#forcast=pd.DataFrame(columns=['increase code','forcast code','relation'])
index =0;
for code1 in np_increase_report:
    for code2 in np_ready_report:
        if cmp(basic.ix[str(code2),'industry'],basic.ix[str(code1),'industry']) == 0:
        	relation = get_relation(str(code1),str(code2))
        	forcast.ix[str(index),'increase code'] = code1
        	forcast.ix[str(index),'forcast code'] = code2
        	forcast.ix[str(index),'relation'] = relation
        	forcast.ix[str(index),'publishDate0'] = np.array(df[df['code'].isin([code2])]['publishDate0'])[0]
        	forcast.ix[str(index),'forcast  industry'] = basic.ix[str(code2),'industry']
        	forcast.ix[str(index),'increase industry'] = basic.ix[str(code1),'industry']
		index = index +1

forcast.to_excel('forcast.xlsx')

