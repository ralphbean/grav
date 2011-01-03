/**
 * @file SessionTreeControl.h
 *
 * Definition of the SessionTreeControl, which is a tree structure for storing
 * audio & video sessions as displayed in the side window.
 * This should be the higher-level client interface for adding/removing/etc
 * sessions.
 *
 * Created on: Sep 13, 2010
 * @author Andrew Ford
 */

#ifndef SESSIONTREECONTROL_H_
#define SESSIONTREECONTROL_H_

#include <wx/treectrl.h>

class SessionManager;

class SessionTreeControl : public wxTreeCtrl
{

public:
    SessionTreeControl();
    SessionTreeControl( wxWindow* parent );
    void setSessionManager( SessionManager* s );

    void addSession( std::string address, bool audio, bool rotate );
    void removeSession( std::string address );
    wxTreeItemId findSession( wxTreeItemId root, std::string address );

    void rotateVideoSessions();

    bool setEncryptionKey( std::string addr, std::string key );
    bool disableEncryption( std::string addr );

    void itemRightClick( wxTreeEvent& evt );
    void addVideoSessionEvent( wxCommandEvent& evt );
    void addAudioSessionEvent( wxCommandEvent& evt );
    void toggleEnableSessionEvent( wxCommandEvent& evt );
    void removeSessionEvent( wxCommandEvent& evt );
    void rotateEvent( wxCommandEvent& evt );
    void setEncryptionEvent( wxCommandEvent& evt );
    void disableEncryptionEvent( wxCommandEvent& evt );

    static int addVideoID;
    static int addAudioID;
    static int toggleEnableID;
    static int removeID;
    static int rotateID;
    static int setEncryptionID;
    static int disableEncryptionID;

private:
    wxTreeItemId rootID;
    wxTreeItemId videoNodeID;
    wxTreeItemId audioNodeID;
    wxTreeItemId rotatedVideoNodeID;

    SessionManager* sessionManager;

    DECLARE_EVENT_TABLE()

};

#endif /* SESSIONTREECONTROL_H_ */