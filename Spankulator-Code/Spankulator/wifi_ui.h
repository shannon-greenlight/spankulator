#include <Wire.h> // For I2C comm, but needed for not getting compile error
#include <TerminalVT100.h>
#include <stdlib.h>
#include <WiFiNINA.h>

void enter_wifi()
{
  selected_fxn = &wifi_fxn;
  selected_fxn->display();
}

String wifi_params_toJSON()
{
  String out = "";
  out += "{ ";
  out += toJSON("label", "Connected to: ");
  out += ", ";
  out += toJSON("type", "text");
  out += ", ";
  out += toJSON("value", String(wifi_ssid.get()));
  out += ", ";
  out += toJSON("selected", "false");
  out += " }";
  out += ",";
  out += "{ ";
  out += toJSON("label", "Status");
  out += ", ";
  out += toJSON("type", "text");
  out += ", ";
  out += toJSON("value", getConnectionStatus(wifi_status));
  out += ", ";
  out += toJSON("selected", "false");
  out += " }";
  out += ",";
  out += "{ ";
  out += toJSON("label", "IP");
  out += ", ";
  out += toJSON("type", "text");
  out += ", ";
  out += toJSON("value", IpAddress2String(WiFi.localIP()));
  out += ", ";
  out += toJSON("selected", "false");
  out += " }";
  out += ",";
  out += "{ ";
  out += toJSON("label", "Signal");
  out += ", ";
  out += toJSON("type", "text");
  out += ", ";
  out += toJSON("value", String(WiFi.RSSI()) + " dBm");
  out += ", ";
  out += toJSON("selected", "false");
  out += " }";
  return "[" + out + "]";
}

String params_toJSON()
{
  switch (fxn.get())
  {
  case LFO_FXN:
    return lfo_params_toJSON();
    break;
  case DVM_FXN:
    return dvm_params_toJSON();
    break;
  case USER_FXN:
    // Serial.println("User params to JSON!");
    return user_params_toJSON();
    break;
  case WIFI_FXN:
    return wifi_params_toJSON();
    break;
  default:
    return selected_fxn->params_toJSON();
  }
}

void send_user_params_to_client(WiFiClient client)
{
  byte temp = user_dig_num;
  String out = "";
  client.print("\"params\" : [");
  for (user_dig_num = 0; user_dig_num < user_string.length() && user_dig_num < USER_MAXLEN; user_dig_num++)
  {
    // Serial.print(user_dig_num);
    // Serial.print(". ");
    // Serial.print(user_string.charAt(user_dig_num));
    // Serial.println("*");
    // Serial.println(out);
    if (user_dig_num > 0)
    {
      out += ",";
    }
    if (user_string.charAt(user_dig_num) != ' ')
    {
      out += user_params_toJSON();
    }
    client.print(out);
    // Serial.print(out);
    out = "";
  }
  client.print("]");
  user_dig_num = temp;
}

void send_data_to_client(WiFiClient client, char cmd)
{
  if (cmd == '[')
    return;
  client.print(toJSON("fxn", fxn_name()));
  client.print(",");
  client.print(toJSON("fxn_num", String(fxn.get())));
  client.print(",");
  if (true || cmd == ' ' || cmd == 'f' || cmd == '+' || cmd == '-')
    client.print(list_fxns());
  client.print(toJSON("device_name", settings_get_device_name()));
  client.print(",");
  client.print(toJSON("quantized", settings_is_quantized() ? "Quantized" : "Normal"));
  client.print(",");
  client.print(toJSON("repeat_on", repeat_on.get() ? "ON" : "OFF"));
  client.print(",");
  client.print(toJSON("triggered", is_triggered ? "ON" : "OFF"));
  client.print(",");
  client.print(toJSON("cmd", String(cmd)));
  client.print(",");
  client.print(toJSON("digit_num", String(get_dig_num())));
  client.print(",");
  client.print(toJSON("param_num", String(get_param_num())));
  client.print(",");
  client.print(toJSON("param_active", String(selected_fxn->get_param_active())));
  client.print(",");
  client.print(toJSON("dp", (selected_fxn->decimal_places) ? String(selected_fxn->decimal_places[selected_fxn->param_num]) : "0"));
  client.print(",");
  client.print(toJSON("adj", String(adj)));
  client.print(",");
  client.print(toJSON("cv_val", String(cv_val)));
  client.print(",");
  // client.print(toJSON("scale", String((int)(scale * ADC_FS + .5))));
  client.print(toJSON("scale", get_control("scale")));
  client.print(",");
  // client.print(toJSON("offset", String(offset_adj)));
  client.print(toJSON("offset", get_control("offset")));
  client.print(",");
  client.print(toJSON("gate", String(gate.get() ? "ON" : "OFF")));
  client.print(",");
  client.print(toJSON("toggle", String(tog.get() ? "ON" : "OFF")));
  client.print(",");
  client.print(toJSON("dac_fs", String(DAC_FS)));
  client.print(",");
  client.print(toJSON("adc_fs", String(ADC_FS)));
  client.print(",");
  client.print(toJSON("clock", !settings_is_ext_clk() ? "internal" : "external"));
  client.print(",");
  client.print(toJSON("trigger_enable", get_trigger_enable() ? "enabled" : "disabled"));
  client.print(",");
  // client.print(toJSON("ext_trigger_disable", get_ext_trigger_disable() ? "disabled" : "enabled"));
  client.print(toJSON("ext_trigger_disable", settings_get_ext_trig() == 0 ? "enabled" : "disabled"));
  client.print(",");

  if (fxn_name() == "User")
  {
    // Serial.println("Sending: "+fxn_name());
    client.print("\"sequence\" : {");
    client.print(toJSON("label", "Sequence"));
    client.print(",");
    client.print(toJSON("type", "sequence"));
    client.print(",");
    client.print(toJSON("value", user_string.get()));
    client.print(",");
    client.print(toJSON("legal_values", user_ops));
    client.print(",");
    client.print(toJSON("dig_num", String(user_dig_num)));
    client.print(",");
    client.print(toJSON("selected", user_entering_param == 0 ? "true" : "false"));
    client.print(",");
    send_user_params_to_client(client);
    client.print("},");
  }

  if (wifi_ui_message > "" || true)
  {
    client.print(toJSON("message", wifi_ui_message));
    client.print(",");
  }

  if (fxn_name() == "Bounce")
  {
    client.print(toJSON("meas", dvm_meas));
    client.print(",");
  }
  else
  {
    // clear out old message
    wifi_ui_message = "";
  }

  client.print("\"params\" : [");
  // fix this for other fxns
  client.print(params_toJSON());
  client.print("]");
}

void _do_server()
{
  char cmd;

  WiFiClient client = server.available(); // listen for incoming clients

  // ui.terminal_debug("Do server. Client: " + String(client));

  if (client)
  { // if you get a client,
    // Serial.println("new client"); // print a message out the serial port
    String currentLine = ""; // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        // Serial.write(c);        // print it out the serial monitor
        if (c == '\n')
        { // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            // client.println("Content-type:application/json");
            client.println("Access-Control-Allow-Origin: *");
            client.println("Access-Control-Allow-Methods: *");
            client.println();

            client.print("{");
            send_data_to_client(client, cmd);
            client.print("}");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else
          { // if you got a newline, then clear currentLine:
            if (strstr(currentLine.c_str(), "POST"))
            {
              cmd = currentLine[6];
              if (cmd == '[')
              {
                esc_mode = true;
                cmd = currentLine[7];
              }

              // Serial.println("Command: " + String(cmd));
              // int c = cmd;
              String in_str = "";
              int i = 0;
              char inchar = currentLine[i + 6];
              while (inchar != ' ' && i < 20)
              {
                in_str += inchar;
                i++;
                inchar = currentLine[i + 6];
              }
              if (in_str == "")
              {
                in_str = " ";
              }
              // Serial.println("Receiving: " + in_str + "*");
              if (in_str.startsWith("-Control"))
              {
                currentLine = "";
              }
              else
              {
                process_cmd(in_str);
              }
            }
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    // Serial.println("client disonnected");
  }
  if (!is_triggered && fxn.get() != LFO_FXN && wifi_status == WL_CONNECTED)
    ui.printWiFiBars(WiFi.RSSI());
}

void do_server()
{
  if (wifi_enabled() && wifi_active.get())
  {
    _do_server();
  }
}
