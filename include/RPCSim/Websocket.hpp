#ifndef RPCSIM_WEBSOCKET_H_
#define RPCSIM_WEBSOCKET_H_

#include "THttpWSHandler.h"
#include "THttpWSHandler.h"
#include "THttpCallArg.h"
#include "TString.h"
#include "TDatime.h"
#include "TTimer.h"

#include <cstdio>

namespace RPCSim
{

class Websocket : public THttpWSHandler {
   public:
      UInt_t fWSId{0};
      Int_t fServCnt{0};

      Websocket(const char *name = nullptr, const char *title = nullptr) : THttpWSHandler(name, title) {}

      // load custom HTML page when open correspondent address
      TString GetDefaultPageContent() override { return "file:/home/working/RPCSimulation/http/ws.htm"; }

      Bool_t ProcessWS(THttpCallArg *arg) override
      {
         if (!arg || (arg->GetWSId() == 0)) return kTRUE;

         // printf("Method %s\n", arg->GetMethod());

         if (arg->IsMethod("WS_CONNECT")) {
            // accept only if connection not established
            return fWSId == 0;
        }

        if (arg->IsMethod("WS_READY")) {
            fWSId = arg->GetWSId();
            printf("Client connected %d\n", fWSId);
            return kTRUE;
        }

        if (arg->IsMethod("WS_CLOSE")) {
           fWSId = 0;
           printf("Client disconnected\n");
           return kTRUE;
        }

        if (arg->IsMethod("WS_DATA")) {
           TString str;
           str.Append((const char *)arg->GetPostData(), arg->GetPostDataLength());
           printf("Client msg: %s\n", str.Data());
           TDatime now;
           SendCharStarWS(arg->GetWSId(), TString::Format("Server replies:%s server counter:%d", now.AsString(), fServCnt++));
           return kTRUE;
        }

        return kFALSE;
      }

      /// per timeout sends data portion to the client
      Bool_t HandleTimer(TTimer *) override
      {
         TDatime now;
         if (fWSId) SendCharStarWS(fWSId, TString::Format("Server sends data:%s server counter:%d", now.AsString(), fServCnt++));
         return kTRUE;
      }

};

}

#endif