void update_graph(){
  if (!bme_ok) return;
  char buffer[10];

  dtostrf(averageValueT.average(), 4, 1, buffer);
  gslc_ElemXGraphAdd(&m_gui,m_pElemGraph_Temperature,(atof(buffer)*4)-30);

  //Serial.print("moltiplicato :");
  //Serial.print((atof(buffer)*4)-30);
  //Serial.print(" non molt. :");
  //Serial.print(atof(buffer)-30);
  //Serial.print(" grezzo :");
  //Serial.print(atof(buffer));
  //Serial.print(" no f :");
  //Serial.println(buffer);

  //dtostrf(averageValueH.average(), 4, 1, buffer);
  //gslc_ElemXGraphAdd(&m_gui,m_pElemGraph_Humidity,(atof(buffer)*4)-30);
  gslc_Update(&m_gui); 
}
