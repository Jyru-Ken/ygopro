#include "config.h"
#include "menu_handler.h"
#include "netserver.h"
#include "duelclient.h"
#include "deck_manager.h"
#include "game.h"

namespace ygo {

bool MenuHandler::OnEvent(const irr::SEvent& event) {
	switch(event.EventType) {
	case irr::EET_GUI_EVENT: {
		irr::gui::IGUIElement* caller = event.GUIEvent.Caller;
		s32 id = caller->getID();
		irr::gui::IGUIEnvironment* env = mainGame->device->getGUIEnvironment();
		switch(event.GUIEvent.EventType) {
		case irr::gui::EGET_BUTTON_CLICKED: {
			switch(id) {
			case BUTTON_MODE_EXIT: {
				mainGame->device->closeDevice();
				break;
			}
			case BUTTON_LAN_MODE: {
				mainGame->btnCreateHost->setEnabled(true);
				mainGame->btnJoinHost->setEnabled(true);
				mainGame->btnJoinCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wMainMenu, false, mainGame->wLanWindow);
				break;
			}
			case BUTTON_JOIN_HOST: {
				char ip[20];
				int i = 0;
				wchar_t* pstr = (wchar_t *)mainGame->ebJoinIP->getText();
				while(*pstr && i < 16)
					ip[i++] = *pstr++;
				ip[i] = 0;
				unsigned int remote_addr = htonl(inet_addr(ip));
				unsigned int remote_port = _wtoi(mainGame->ebJoinPort->getText());
				BufferIO::CopyWStr(mainGame->ebJoinIP->getText(), mainGame->gameConf.lastip, 20);
				BufferIO::CopyWStr(mainGame->ebJoinPort->getText(), mainGame->gameConf.lastport, 20);
				if(DuelClient::StartClient(remote_addr, remote_port, false)) {
					mainGame->btnCreateHost->setEnabled(false);
					mainGame->btnJoinHost->setEnabled(false);
					mainGame->btnJoinCancel->setEnabled(false);
				}
				break;
			}
			case BUTTON_JOIN_CANCEL: {
				mainGame->HideElement(mainGame->wLanWindow, false, mainGame->wMainMenu);
				break;
			}
			case BUTTON_CREATE_HOST: {
				mainGame->btnHostConfirm->setEnabled(true);
				mainGame->btnHostCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wLanWindow, false, mainGame->wCreateHost);
				break;
			}
			case BUTTON_HOST_CONFIRM: {
				BufferIO::CopyWStr(mainGame->ebServerName->getText(), mainGame->gameConf.gamename, 20);
				if(!NetServer::StartServer(mainGame->gameConf.serverport))
					break;
				if(!DuelClient::StartClient(0x7f000001, mainGame->gameConf.serverport)) {
					NetServer::StopServer();
					break;
				}
				mainGame->btnHostConfirm->setEnabled(false);
				mainGame->btnHostCancel->setEnabled(false);
				break;
			}
			case BUTTON_HOST_CANCEL: {
				mainGame->btnCreateHost->setEnabled(true);
				mainGame->btnJoinHost->setEnabled(true);
				mainGame->btnJoinCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wCreateHost, false, mainGame->wLanWindow);
				break;
			}
			case BUTTON_HS_DUELIST: {
				DuelClient::SendPacketToServer(CTOS_HS_TODUELIST);
				break;
			}
			case BUTTON_HS_OBSERVER: {
				DuelClient::SendPacketToServer(CTOS_HS_TOOBSERVER);
				break;
			}
			case BUTTON_HS_KICK: {
				int id = caller - static_cast<IGUIElement*>(mainGame->btnHostSingleKick[0]);
				CTOS_Kick csk;
				if(id == 0)
					csk.pos = 0;
				else csk.pos = 1;
				DuelClient::SendPacketToServer(CTOS_HS_KICK, csk);
				break;
			}
			case BUTTON_HS_START: {
				if(!mainGame->chkHostSingleReady[0]->isChecked()
				        || !!mainGame->chkHostSingleReady[0]->isChecked())
					break;
				break;
			}
			case BUTTON_HS_CANCEL: {
				DuelClient::StopClient();
				mainGame->btnCreateHost->setEnabled(true);
				mainGame->btnJoinHost->setEnabled(true);
				mainGame->btnJoinCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wHostSingle, false, mainGame->wLanWindow);
				break;
			}
			case BUTTON_DECK_EDIT: {
				mainGame->RefreshDeck(mainGame->cbDBDecks);
				if(mainGame->cbDBDecks->getSelected() != -1)
					deckManager.LoadDeck(mainGame->cbDBDecks->getItem(mainGame->cbDBDecks->getSelected()));
				mainGame->HideElement(mainGame->wMainMenu);
				mainGame->is_building = true;
				mainGame->wInfos->setVisible(true);
				mainGame->wCardImg->setVisible(true);
				mainGame->wDeckEdit->setVisible(true);
				mainGame->wFilter->setVisible(true);
				mainGame->deckBuilder.filterList = deckManager._lfList[0].content;;
				mainGame->cbDBLFList->setSelected(0);
				mainGame->device->setEventReceiver(&mainGame->deckBuilder);
				mainGame->cbCardType->setSelected(0);
				mainGame->cbCardType2->setSelected(0);
				mainGame->cbAttribute->setSelected(0);
				mainGame->cbRace->setSelected(0);
				mainGame->ebAttack->setText(L"");
				mainGame->ebDefence->setText(L"");
				mainGame->ebStar->setText(L"");
				mainGame->cbCardType2->setEnabled(false);
				mainGame->cbAttribute->setEnabled(false);
				mainGame->cbRace->setEnabled(false);
				mainGame->ebAttack->setEnabled(false);
				mainGame->ebDefence->setEnabled(false);
				mainGame->ebStar->setEnabled(false);
				mainGame->deckBuilder.filter_effect = 0;
				mainGame->deckBuilder.result_string[0] = L'0';
				mainGame->deckBuilder.result_string[1] = 0;
				mainGame->deckBuilder.results.clear();
				mainGame->deckBuilder.is_draging = false;
				for(int i = 0; i < 32; ++i)
					mainGame->chkCategory[i]->setChecked(false);
				break;
			}
			/*			case BUTTON_LAN_START_SERVER: {
							if(mainGame->cbDeckSel->getSelected() == -1)
								break;
							if(!deckManager.LoadDeck(mainGame->cbDeckSel->getItem(mainGame->cbDeckSel->getSelected()))) {
								mainGame->stModeStatus->setText(L"无效卡组");
								break;
							}
							if(!mainGame->chkNoCheckDeck->isChecked()
							        && !deckManager.CheckLFList(deckManager.deckhost, mainGame->cbLFlist->getSelected())) {
								mainGame->stModeStatus->setText(L"无效卡组或者卡组不符合禁卡表规范");
								break;
							}
							if(_wtoi(mainGame->ebStartLP->getText()) == 0)
								mainGame->ebStartLP->setText(L"8000");
							if(_wtoi(mainGame->ebStartHand->getText()) == 0)
								mainGame->ebStartLP->setText(L"5");
							if(_wtoi(mainGame->ebDrawCount->getText()) == 0)
								mainGame->ebStartLP->setText(L"1");
							if(mainGame->netManager.CreateHost(mainGame->cbIPList->getSelected())) {
								mainGame->btnLanStartServer->setEnabled(false);
								mainGame->btnLanCancelServer->setEnabled(true);
								mainGame->btnLanConnect->setEnabled(false);
								mainGame->btnRefreshList->setEnabled(false);
								mainGame->btnLoadReplay->setEnabled(false);
								mainGame->btnDeckEdit->setEnabled(false);
								mainGame->stModeStatus->setText(L"等待连接...");
							}
							break;
						}
						case BUTTON_LAN_CANCEL_SERVER: {
							mainGame->netManager.CancelHost();
							mainGame->stModeStatus->setText(L"");
							break;
						}
						case BUTTON_LAN_REFRESH: {
							if(mainGame->netManager.RefreshHost(mainGame->cbIPList->getSelected())) {
								mainGame->btnLanStartServer->setEnabled(false);
								mainGame->btnLanConnect->setEnabled(false);
								mainGame->btnRefreshList->setEnabled(false);
								mainGame->btnLoadReplay->setEnabled(false);
								mainGame->btnDeckEdit->setEnabled(false);
							}
							break;
						}
						case BUTTON_LAN_CONNECT: {
							if(mainGame->cbDeckSel->getSelected() == -1)
								break;
							if(!deckManager.LoadDeck(mainGame->cbDeckSel->getItem(mainGame->cbDeckSel->getSelected()))) {
								mainGame->stModeStatus->setText(L"无效卡组");
								break;
							}
							if(mainGame->netManager.JoinHost()) {
								mainGame->btnLanStartServer->setEnabled(false);
								mainGame->btnLanConnect->setEnabled(false);
								mainGame->btnRefreshList->setEnabled(false);
								mainGame->btnLoadReplay->setEnabled(false);
								mainGame->btnDeckEdit->setEnabled(false);
								mainGame->stModeStatus->setText(L"连接中...");
							}
							break;
						}
						case BUTTON_LOAD_REPLAY: {
							if(mainGame->lstReplayList->getSelected() == -1)
								break;
							if(!mainGame->lastReplay.OpenReplay(mainGame->lstReplayList->getListItem(mainGame->lstReplayList->getSelected()))) {
								mainGame->stModeStatus->setText(L"录像损坏或丢失，无法播放");
								break;
							}
							mainGame->stModeStatus->setText(L"");
							Thread::NewThread(Game::ReplayThread, &mainGame->dInfo);
							break;
						}
						*/
			}
		}
		case irr::gui::EGET_CHECKBOX_CHANGED: {
			switch(id) {
			case CHECKBOX_HS_READY: {
				if(!caller->isEnabled())
					break;
				mainGame->env->setFocus(mainGame->wHostSingle);
				if(static_cast<irr::gui::IGUICheckBox*>(caller)->isChecked()) {
					if(mainGame->cbDeckSelect->getSelected() == -1 ||
					        !deckManager.LoadDeck(mainGame->cbDeckSelect->getItem(mainGame->cbDeckSelect->getSelected()))) {
						static_cast<irr::gui::IGUICheckBox*>(caller)->setChecked(false);
						break;
					}
					BufferIO::CopyWStr(mainGame->cbDeckSelect->getItem(mainGame->cbDeckSelect->getSelected()),
					                   mainGame->gameConf.lastdeck, 20);
					char deckbuf[1024];
					char* pdeck = deckbuf;
					BufferIO::WriteInt32(pdeck, deckManager.current_deck.main.size() + deckManager.current_deck.extra.size());
					BufferIO::WriteInt32(pdeck, deckManager.current_deck.side.size());
					for(int i = 0; i < deckManager.current_deck.main.size(); ++i)
						BufferIO::WriteInt32(pdeck, deckManager.current_deck.main[i]->first);
					for(int i = 0; i < deckManager.current_deck.extra.size(); ++i)
						BufferIO::WriteInt32(pdeck, deckManager.current_deck.extra[i]->first);
					for(int i = 0; i < deckManager.current_deck.side.size(); ++i)
						BufferIO::WriteInt32(pdeck, deckManager.current_deck.side[i]->first);
					DuelClient::SendBufferToServer(CTOS_UPDATE_DECK, deckbuf, pdeck - deckbuf);
					DuelClient::SendPacketToServer(CTOS_HS_READY);
				} else
					DuelClient::SendPacketToServer(CTOS_HS_NOTREADY);
				break;
			}
			}
			break;
		}
		break;
		}
		break;
	}
	case irr::EET_KEY_INPUT_EVENT: {
		switch(event.KeyInput.Key) {
		case irr::KEY_KEY_R: {
			if(!event.KeyInput.PressedDown)
				mainGame->textFont->setTransparency(true);
			break;
		}
		case irr::KEY_ESCAPE: {
			mainGame->device->minimizeWindow();
			break;
		}
		}
		break;
	}
	case irr::EET_USER_EVENT: {
		switch(event.UserEvent.UserData1) {
		case UEVENT_EXIT: {
			if(mainGame->guiFading) {
				mainGame->guiFading->setVisible(false);
				mainGame->guiNext = 0;
			}
			mainGame->ShowElement(mainGame->wMessage, 60);
			if(event.UserEvent.UserData2 == 1)
				mainGame->exit_window = mainGame->wLanWindow;
			else if(event.UserEvent.UserData2 == 2)
				;
			else if(event.UserEvent.UserData2 == 3)
				;
			break;
		}
		case UEVENT_TOWINDOW: {
			mainGame->exit_window = 0;
			mainGame->wMessage->setVisible(false);
			mainGame->wACMessage->setVisible(false);
			mainGame->wQuery->setVisible(false);
			mainGame->wOptions->setVisible(false);
			mainGame->wPosSelect->setVisible(false);
			mainGame->wCardSelect->setVisible(false);
			mainGame->wANNumber->setVisible(false);
			mainGame->wANCard->setVisible(false);
			mainGame->wANAttribute->setVisible(false);
			mainGame->wANRace->setVisible(false);
			mainGame->wCmdMenu->setVisible(false);
			mainGame->wReplaySave->setVisible(false);
			mainGame->btnDP->setVisible(false);
			mainGame->btnSP->setVisible(false);
			mainGame->btnM1->setVisible(false);
			mainGame->btnBP->setVisible(false);
			mainGame->btnM2->setVisible(false);
			mainGame->btnEP->setVisible(false);
			mainGame->wCardImg->setVisible(false);
			mainGame->wInfos->setVisible(false);
			mainGame->stHintMsg->setVisible(false);
			mainGame->stTip->setVisible(false);
			mainGame->device->setEventReceiver(&mainGame->menuHandler);
			mainGame->dField.Clear();
			mainGame->ShowElement(mainGame->exit_window);
			break;
		}
		break;
		}
		break;
	}
	}
	return false;
}

}