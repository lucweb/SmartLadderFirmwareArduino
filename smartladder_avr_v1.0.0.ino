#include <EEPROM.h>

String _P, _F, _R, _TD;

int _CT[10], _ST;
long int _TL[10];
byte _B[20], _TPR, _CCCTU[10], _CCCTD[10];

void setup()
{
  Serial.begin(115200);
  delay(2);
  loadPort();
  typePort();
  loadProgram();
}


void declare(int port, char tipePort)
{
  if (tipePort == 'i')
    pinMode(port, INPUT);
  else if (tipePort == 'o')
    pinMode(port, OUTPUT);
  delay(0.5);
}

void typePort()
{
  String p = "";
  for (int x = 0; x < _R.length(); x++)
  {
    if (_R[x] != 'i' && _R[x] != 'o')
      p += _R[x];
    else
    {
      declare(p.toInt(), _R[x]);
      p = "";
    }
  }

  upForce();
}

int pEEPROM()
{
  return _R.length() + 2;
}

void loadProgram()
{
  _P = "";
  int q = pEEPROM();
  for (int i = q; i < 1024; i++)
  {
    char d = (char)EEPROM.read(i);
    if (d == '#' || d == '\0')
      return;
    _P += d;
  }
  delay(5);
  Serial.print(_P);
}

void loadPort()
{
  _R = "";
  for (int i = 0; i < 1024; i++)
  {
    char d = (char)EEPROM.read(i);
    if (d == '#' || d == '\0' || (d != '0' && !d))
      return;
    _R += d;
  }
  delay(5);
  typePort();
  Serial.print(_R);
}

void setEEPROMProgram()
{
  int q = pEEPROM();
  int c = _P.length() + q;
  for (int i = q; i < c; i++)
    EEPROM.write(i, _P[i - q]);
  EEPROM.write(c, '#');
}

void setEEPROMPort()
{
  int c = _R.length();
  for (int i = 0; i < c; i++)
    EEPROM.write(i, _R[i]);
  EEPROM.write(c, '#');
  typePort();
}

bool isForce(int i, char tipo = 0)
{
  String fMsg = "";
  for (int x = 0; x < _F.length(); x++)
  {
    if (_F[x] != ';')
      fMsg += _F[x];
    else
    {
      fMsg.remove(0, 1);
      String tp = (tipo == 0) ? "" : String(tipo);
      if (fMsg == "/" + tp + String(i))
        return 1;
      fMsg = "";
    }
  }
  return 0;
}

void setForce(String m)
{
  String b = "";
  for (int i = 2; i < m.length(); i++)
    b += m[i];
  int isBit = b.indexOf('b');
  if (isBit >= 0)
  {
    b.remove(0, 1);
    _B[b.toInt()] = m[0];
  }
  else
    digitalWrite(b.toInt(), String(m[0]).toInt());
}

void upForce()
{
  String _FMsg = "";
  for (int x = 0; x < _F.length(); x++)
  {
    if (_F[x] != ';')
      _FMsg += _F[x];
    else
    {
      setForce(_FMsg);
      _FMsg = "";
    }
  }
}

int getCount(int i)
{
  return abs(_CT[i]);
}

bool setCount(int i, int t)
{
  if (_CT[i] >= 0 && _CT[i] < t && !_CCCTU[i])
  {
    _CCCTU[i] = 1;
    _CT[i]++;
    if (_CT[i] == t)
      _CT[i] = -_CT[i];
  }
  return getCount(i) == t;
}

void resetCount(int i)
{
  _CT[i] = 0;
}

int setCountDown(int i, int t)
{
  if (getCount(i) != 0 && !_CCCTD[i])
  {
    _CCCTD[i] = 1;
    _CT[i] = getCount(i) - t;
    if (_CT[i] < 1)
      _CT[i] = 0;
  }
  return getCount(i) == 0;
}

int getTemp(int x)
{
  return  _TL[x] > 0 ? (millis() - _TL[x]) : (_TL[x] < 0 ? abs(_TL[x]) : 0);
}

void loopTemp(int t, int qt)
{
  if (millis() - _TL[t] >= qt)
    _TL[t] = -qt;
}

void resetTemp(int t)
{
  _TL[t] = 0;
}

bool validaTemp(int t, int qt)
{
  if (_TL[t] >= 0)
  {
    if (_TL[t] == 0)
      _TL[t] = millis();
    loopTemp(t, qt);
  }
  return (_TL[t] < 0);
}

int atvIADCPort(int resource)
{
  return analogRead(resource);
}

int atvIDACPort(int resource, String value)
{
  int p = 0;
  String v1 = "";
  String v2 = "";
  String v3 = "";
  for (int i = 0; i < value.length(); i++)
  {
    if (value[i] == '/')
      p++;
    else
    {
      if (p == 0)
        v1 += value[i];
      if (p == 1)
        v2 += value[i];
      if (p == 2)
        v3 += value[i];
    }
  }

  int tempValue = 0;
  if (v1[0] == 'A')
  {
    v1.remove(0, 1);
    tempValue = atvIADCPort(v1.toInt());
  }
  else if (v1[0] == 'c')
  {
    v1.remove(0, 1);
    tempValue = getCount(v1.toInt());
  }
  else
  {
    tempValue = v1.toInt();
  }

  int result = map(tempValue, v2.toInt(), v3.toInt(), 0, 255);
  analogWrite(resource, result);
  return result;
}

bool atvIOPort(int resource, int tCond, int s)
{
  switch (tCond)
  {
  case 1:
    return digitalRead(resource);
    break;
  case 2:
    return !digitalRead(resource);
    break;
  case 3:
    if (!isForce(resource))
      digitalWrite(resource, s);
    break;
  case 4:
    if (!isForce(resource))
    {
      if (s)
        digitalWrite(resource, 1);
    }
    break;
  case 5:
    if (!isForce(resource))
    {
      if (s)
        digitalWrite(resource, 0);
    }
    break;
  }
  return s;
}

bool atvIOBit(String port, int tCond, int s)
{
  port.remove(0, 1);
  int b = port.toInt();
  switch (tCond)
  {
  case 1:
    return _B[b];
    break;
  case 2:
    return !_B[b];
    break;
  case 3:
    if (!isForce(b, 'b'))
    {
      _B[b] = s;
    }
    break;
  case 4:
    if (!isForce(b, 'b'))
    {
      if (s)
        _B[b] = 1;
    }
    break;
  case 5:
    if (!isForce(b, 'b'))
    {
      if (s)
      {
        _B[b] = 0;
      }
    }
    break;
  }
  return s;
}

bool atvTime(String port, int tCond, String prop, int s)
{
  port.remove(0, 1);
  int t = port.toInt();
  int value = prop.toInt();
  switch (tCond)
  {
  case 1:
    return _TL[t] < 0;
    break;
  case 2:
    return _TL[t] >= 0;
    break;
  case 6:
    if (s)
      return validaTemp(t, value);
    else if (_TL[t] > 0)
    {
      loopTemp(t, value);
    }
    return s;
    break;
  case 9:
    if (s)
      resetTemp(t);
    break;
  }
}

bool atvCount(String port, int tCond, String prop, int s)
{
  port.remove(0, 1);
  int t = port.toInt();
  if (tCond == 9)
  {
    if (s)
      resetCount(t);
  }
  switch (tCond)
  {
  case 1:
    return _CT[t] < 0;
    break;
  case 2:
    return _CT[t] >= 0;
    break;
  case 7:
    bool r = (s) ? setCount(t, prop.toInt()) : 0;
    _CCCTU[t] = (!s) ? 0 : 1;
    return r;
    break;
  }
}

bool atvCountDown(String port, int tCond, String prop, int s)
{
  port.remove(0, 1);
  int t = port.toInt();

  bool r = (s) ? setCountDown(t, prop.toInt()) : 0;
  _CCCTD[t] = (!s) ? 0 : 1;
  return r;
}

bool atv(String tipoCond, String port, String prop, int s)
{
  int tCond = tipoCond.toInt();
  int resource = port.toInt();

  if (tCond == 10)
  {
    atvIADCPort(resource);
    return s;
  }
  else if (tCond == 11)
  {
    atvIDACPort(resource, prop);
    return s;
  }
  else if (tCond == 12)
  {
    return atvCountDown(port, tCond, prop, s);
  }
  else if (resource && tCond != 0)
    return atvIOPort(resource, tCond, s);
  else if (port[0] == 'b')
    return atvIOBit(port, tCond, s);
  else if (port[0] == 'A')
  {
  }
  else if (port[0] == 'a')
  {
  }
  else if (port[0] == 'T')
    return atvTime(port, tCond, prop, s);
  else if (tCond > 0 && port[0] == 'c')
  {
    return atvCount(port, tCond, prop, s);
  }
  else if (tCond == 0)
    return setEqual(tipoCond, port, prop);
  return true;
}

int checkStatus(String port)
{
  int resource = port.toInt();
  if (resource)
    digitalRead(resource);
  else if (port[0] == 'b')
  {
    port.remove(0, 1);
    int t = port.toInt();
    return _B[t];
  }
  else if (port[0] == 'T')
  {
    port.remove(0, 1);
    int t = port.toInt();
    return getTemp(t);
  }
  else if (port[0] == 'c')
  {
    port.remove(0, 1);
    int t = port.toInt();
    return getCount(t);
  }
  return 0;
}

bool setEqual(String tipoCond, String sA, String sB)
{
  int iA = sA.indexOf('/');
  int iB = sB.indexOf('/');
  int a = 0;
  int b = 0;
  if (iA >= 0)
  {
    sA.remove(0, 1);
    a = checkStatus(sA);
  }
  else
    a = sA.toInt();

  if (iB >= 0)
  {
    sB.remove(0, 1);
    b = checkStatus(sB);
  }
  else
    b = sB.toInt();

  switch (tipoCond[0])
  {
  case '=':
    return (a == b);
    break;
  case '!':
    return (a != b);
    break;
  case '-':
    return (a < b);
    break;
  case '+':
    return (a > b);
    break;
  }
}

void start()
{
  byte h = 0;
  String a = "";
  String b = "";
  String c = "";
  int x = 0;
  bool r = 0;

  for (int i = 0; i < _P.length(); i++)
  {
    if (_P[i] == '>')
      continue;
    if (_P[i] == 'l')
      r = 1;
    else if (_P[i] == ':')
      x++;
    else if (_P[i] != ';' && _P[i] != '_')
    {
      if (x == 0)
        a += _P[i];
      else if (x == 1)
        b += _P[i];
      else if (x == 2)
        c += _P[i];
    }
    else
    {
      if (h != 0 && _P[i] == ';')
      {
        if (!r && h == 2)
          r = 1;
        else if (r && h == 1)
          r = 0;
        h = 0;
      }

      if (a != "")
      {
        bool s = atv(a, b, c, r);
        if (r && _P[i] != '_')
          r = s;

        if (r && _P[i] == '_' && h < 2)
          h = (s) ? 2 : 1;
      }
      a = "";
      b = "";
      c = "";
      x = 0;
    }
  }
}

void startTypeRequest(char e)
{
  if (e == '#')
  {
    _P = "";
    _TPR = 1; 
  }
  else if (e == '@')
  {
    _P = "";
    _TPR = 5; 
  }
  else if (e == '$')
  {
    _F = "";
    _TPR = 2; 
  }
  else if (e == '|')
  {
    _R = "";
    _TPR = 3; 
  }
  else if (e == '&')
  {
    _TPR = 4;
  }
  else if (e == '*')
  { 
    _R = "";
    _TPR = 6;
  }
  else if (e == 's')
  {
    _TPR = 7;
  }
  else if (e == 'c')
  {
    _TPR = 8;
  }
}

void eR()
{
  if (_TPR == 1)
    Serial.print(String('r') + String('p') + String(_P.length()));
  else if (_TPR == 2)
  {
    upForce();
    Serial.print(String('r') + String('f') + String(_F.length()));
  }
  else if (_TPR == 3)
  {
    typePort();
    Serial.print(String('r') + String('r') + String(_R.length()));
  }
  else if (_TPR == 4)
  {
    _ST = _TD.toInt();
    _TD = "";
  }
  else if (_TPR == 5)
  {
    setEEPROMProgram();
    Serial.print(String('r') + String('e') + String(_P.length()));
  }
  else if (_TPR == 6)
  {
    typePort();
    setEEPROMPort();
    Serial.print(String(F("r_P")) + String(_R.length()));
  }
  else if (_TPR == 8)
  {
    Serial.print(1);
  }
  _TPR = 0;
}

void receiver(char e)
{
  if (e == '.')
    eR();
  else if (_TPR == 1 || _TPR == 5)
    _P += e;
  else if (_TPR == 2)
    _F += e;
  else if (_TPR == 3 || _TPR == 6)
    _R += e;
  else if (_TPR == 4)
    _TD += e;
  else
    startTypeRequest(e);
}

void emitDigitalRead()
{
  String p = "";
  String v = ">s";
  String va = "";
  for (int x = 0; x < _R.length(); x++)
  {
    if (_R[x] != 'i' && _R[x] != 'o' && _R[x] != 'a')
      p += _R[x];
    else if (_R[x] == 'a')
    {
      va += String(atvIADCPort(p.toInt())) + ':';
      p = "";
    }
    else
    {
      v += String(digitalRead(p.toInt()));
      p = "";
    }
  }
  Serial.print(v + '-');
  Serial.print(va + '-');
}

void setStatusResource()
{
  _ST = 0;
  emitDigitalRead();
  
  for (int x = 0; x < 10; x++)
    Serial.print(String(getTemp(x)) + ':');

  Serial.print('-');
  for (int x = 0; x < 10; x++)
    Serial.print(String(getCount(x)) + ':');

  Serial.print('-');
  for (int x = 0; x < 20; x++)
    Serial.print(_B[x] ? 1 : 0);

  Serial.print('<');
}

void loop()
{
  while (Serial.available())
    receiver(Serial.read());

  if (_TPR == 0)
    start();

  if (_TPR == 0 && _ST > 0)
    setStatusResource();
}
