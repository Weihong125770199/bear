import tushare as ts
import numpy as np
import pandas as pd
from pandas import  Series,DataFrame
#rest=ts.get_industry_classified()
#rest.to_excel('stock.xlsx')
#base=ts.get_today_all()
#base.to_excel('base.xlsx')
#hist_data = ts.get_hist_data('000060',start='2017-07-01')
#hist_data.to_excel('000060.xlsx')
#cpi=ts.get_cpi()
#cpi.to_excel('cpi.xlsx')
##df=ts.profit_data(top=60)
##df.sort('shares',ascending=False)
##df.to_excel('profit_df.xlsx')

#
#yj=ts.forecast_data(2018,2)
#np_data = np.array(yj)
#code = np_data[:,0]
#for a in np.array(code):
#        hist_data = ts.get_hist_data(str(a),start='2017-07-01')
#        hist_data.sort_values(by = "date",ascending = True,inplace = True)
#        if hist_data is not None:
#                np_hist_data = np.array(hist_data)
#                close_price  = np_hist_data[:,3]
#             	hist_data.to_excel(str(a)+'.xlsx')

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


basic = ts.get_stock_basics()
#basic.to_excel('basics.xlsx')
np_data = np.array(basic.index)
#print np_data
a = np.zeros((0,0))
print a
result = pd.DataFrame(a)
#result.to_excel('sample.xlsx')
total = np_data.size
current_size =0
for code in np_data:
    current_size =0
    for current_code  in np_data:
        cor=get_relation(str(code),str(current_code))
        result.ix[str(code),str(current_code)] = cor
        current_size = current_size +1
        rate =current_size/total
        print('curret =%d ,total =%d \n',current_size,total)
    result.to_excel(str(code) +'_sample.xlsx')
        
#result= get_relation('000063','600380')

#print result

#print first_num
#print second_num
#print type(hist_data.index)
#np_first  = np.array(hist_data.close)

#np_first  = np.array(hist_data.close)

#print np_first
#np_second = np.array(hist_data_second.close)

#print np_second
#c = np.vstack([np_first,np_second])

#print type(c)
#print np.corrcoef(c)
#np_data = np.array(hist_data)
#arry = np_data.as_matrix();

#print arry
#hist_data_t =  hist_data.transpose()
#hist_data.insert(0,'code','000063')
#hist_data_t.to_excel('300556.xlsx')
#np_hist_data = np.array(hist_data)
#a = '000063'
#new = np.insert(np_hist_data,0,values = a,axis =1)
#print np_hist_data
#print new
#close_price
#close_price.T
#print close_price.shape


#df = DataFrame(close_price)
#print close_price
#df.to_excel('000063_df.xlsx')

#hist_data = ts.get_hist_data('300554',start='2017-07-01')
#print hist_data
#hist_data.to_excel('300554.xlsx')

#print type(np_data)
#yj.to_excel('yeji22333.xlsx')
#free_data=ts.xsg_data()
#free_data.to_excel('jiejin.xlsx')
#fund_holding=ts.fund_holdings(2018,2)
#fund_holding.to_excel('fund_holding.xlsx')
#basic = ts.get_stock_basics()
#basic.to_excel('basics.xlsx')
#report_data = ts.get_report_data(2018,2)
#report_data.to_excel('report_data.xlsx')

