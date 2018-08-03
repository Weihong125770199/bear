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
for code in np.array(basic.index):
    current_price = ts.get_realtime_quotes(str(code))
    
    basic.ix[str(code),'price'] = np.array(current_price['price'])[0]
    #print type( basic.ix[str(code),'esp'])
    #print type( np.array(current_price['price'])[0])
    basic.ix[str(code),'per-price-eps'] =  basic.ix[str(code),'esp'] /float(np.array(current_price['price'])[0])
basic = basic[~basic['price'].isin(['0.000'])]
basic.sort_values(by = "per-price-eps",ascending = True,inplace = True)
basic.to_excel( year+month+day+second + 'basics_price.xlsx')
