import tushare as ts
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
##yj=ts.forecast_data(2018,2)
#yj.to_excel('yeji22.xlsx')
#free_data=ts.xsg_data()
#free_data.to_excel('jiejin.xlsx')
fund_holding=ts.fund_holdings(2018,2)
fund_holding.to_excel('fund_holding.xlsx')


