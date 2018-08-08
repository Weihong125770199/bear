# -*- coding:utf-8 -*
import tushare as ts
import numpy as np
import pandas as pd
import datetime
import chardet
from pandas import  Series,DataFrame
#rest=ts.get_industry_classified()
#rest.to_excel('stock.xlsx')
#base=ts.get_today_all()
#base.to_excel('base.xlsx')
#hist_data = ts.get_hist_data('000671',start='2017-07-01')
#hist_data.to_excel('000671.xlsx')
#cpi=ts.get_cpi()
#cpi.to_excel('cpi.xlsx')
#df=ts.profit_data(2018,2)
#df.sort('shares',ascending=False)
#df.to_excel('profit_df.xlsx')

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
#print month
day =  datetime.datetime.now().strftime('%d')
second = datetime.datetime.now().strftime('%s')
season = int(month) /3 +1
print season
basic = ts.get_stock_basics()
basic.to_excel( year+month+day+second + '_basics.xlsx')

grouped_pe = basic['pe'].groupby(basic['industry'])
#print grouped.mean()
grouped_pe.mean().to_excel( year+month+day+second + '_grouped_pe.xlsx')

grouped_pb = basic['pb'].groupby(basic['industry'])
#print grouped.mean()
grouped_pb.mean().to_excel( year+month+day+second + '_grouped_pb.xlsx')


grouped_industry=pd.concat([grouped_pe.mean(),grouped_pb.mean()],axis =1 ,join = 'inner')
grouped_industry.to_excel( year+month+day+second + '_grouped_industry.xlsx')


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
           		hist_data.to_excel( year+month+day+second+str(code) + 'history.xlsx')
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
                                result_select.ix[str(code),'pray-values'] = \
                             result_select.ix[str(code),'price-values'] * result_select.ix[str(code),'npr']/100.0

    
#result_select = result[result['type'].isin([u'\u9884\u5347'])]
#print result_select
#result_select.dropna(axis=1,how = 'any')
result_select.to_excel( year+month+day+second + '_yeji.xlsx')

#basic_select = basic[basic.index.isin(['600114','600116','600173','600193','600230','600569','601339','601678','601966','603636','603658'])]
#basic_select.to_excel( year+month+day+second + 'base_select.xlsx')

#select_increase_industry = basic_select[basic_select['industry'].isin(np.array(result_select['industry']))]
#if select_increase_industry is not None:
#   select_increase_industry.to_excel( year+month+day+second + 'same_industry.xlsx')




#np_data = np.array(result_select.index)
#np_tomorrow_data = np.array(basic_select.index)
#print np_data
#a = np.zeros((0,0))
#print a
#result = pd.DataFrame(a)
#result.to_excel('sample.xlsx')
#total = np_data.size
#current_size =0
#for code in np_data:
#    current_size =0
#    for current_code  in np_tomorrow_data:
#        cor=get_relation(str(code),str(current_code))
#        result.ix[str(code),str(current_code)] = cor
#        current_size = current_size +1
#        rate =current_size/total
#        print('curret =%d ,total =%d \n',current_size,total)
#result.to_excel(str(code) +'_tomorrow.xlsx')



#np_data = np.array(yj.code)
#for code in np_data:
#    temp = basic['code']
#    print temp     
#print np_data

#free_data=ts.xsg_data()
#free_data.to_excel('jiejin.xlsx')
#fund_holding=ts.fund_holdings(2018,2)
#fund_holding.to_excel('fund_holding.xlsx')
#basic = ts.get_stock_basics()
#basic.to_excel('basics.xlsx')
#report_data = ts.get_report_data(2018,2)
#report_data.to_excel('report_data.xlsx')
#growth_data = ts.get_growth_data(2018,2)
#growth_data.to_excel('growth_data.xlsx')

