#ifndef VCSPOTIFY_H_
#define VCSPOTIFY_H_

#include "vcplugin.h"
#include "networkinterface.h"

class VCSpotify final : public VCPlugin
{
    Q_OBJECT
    Q_PROPERTY( QString userName            READ userName                                    NOTIFY userNameChanged )
    Q_PROPERTY( QString userEmail           READ userEmail                                   NOTIFY userEmailChanged )
    Q_PROPERTY( QString userSubscription    READ userSubscription                            NOTIFY userSubscriptionChanged )
    Q_PROPERTY( QUrl userImage              READ userImage                                   NOTIFY userImageChanged )
    Q_PROPERTY( bool isActive               READ isActive                                    NOTIFY isActiveChanged )
    Q_PROPERTY( bool isPlaying              READ isPlaying                                   NOTIFY isPlayingChanged )
    Q_PROPERTY( bool shuffleEnabled         READ shuffleEnabled                              NOTIFY shuffleEnabledChanged )
    Q_PROPERTY( bool repeatOneEnabled       READ repeatOneEnabled                            NOTIFY repeatOneEnabledChanged )
    Q_PROPERTY( bool repeatAllEnabled       READ repeatAllEnabled                            NOTIFY repeatAllEnabledChanged )
    Q_PROPERTY( QString trackName           READ trackName                                   NOTIFY trackNameChanged )
    Q_PROPERTY( QString trackArtist         READ trackArtist                                 NOTIFY trackArtistChanged )
    Q_PROPERTY( QString trackAlbum          READ trackAlbum                                  NOTIFY trackAlbumChanged )
    Q_PROPERTY( QUrl trackAlbumArt          READ trackAlbumArt                               NOTIFY trackAlbumArtChanged )
    Q_PROPERTY( int trackPosition           READ trackPosition                               NOTIFY trackPositionChanged )
    Q_PROPERTY( int trackDuration           READ trackDuration                               NOTIFY trackDurationChanged )
    Q_PROPERTY( QString playlistName        READ playlistName                                NOTIFY playlistNameChanged )
    Q_PROPERTY( QVariantList playlists      READ playlists                                   NOTIFY playlistsChanged )
    Q_PROPERTY( QString deviceName          READ deviceName                                  NOTIFY deviceNameChanged )
    Q_PROPERTY( QString deviceType          READ deviceType                                  NOTIFY deviceTypeChanged )
    Q_PROPERTY( int deviceVolume            READ deviceVolume                                NOTIFY deviceVolumeChanged )
    Q_PROPERTY( QVariantList devices        READ devices                                     NOTIFY devicesChanged )
    Q_PROPERTY( QString preferredDevice     READ preferredDevice   MEMBER preferredDevice_  NOTIFY preferredDeviceChanged )
    Q_PROPERTY( QVariantList searchResults  READ searchResults                               NOTIFY searchResultsChanged )
    Q_PROPERTY( QString clientID            MEMBER clientID_                                NOTIFY clientIDChanged )
    Q_PROPERTY( QString clientSecret        MEMBER clientSecret_                            NOTIFY clientSecretChanged )
    Q_PROPERTY( QString refreshToken        MEMBER refreshToken_                            NOTIFY refreshTokenChanged )

public:
    explicit VCSpotify( const QString & name, QObject * parent = nullptr );

    const QString & userName() const { return userName_; }
    const QString & userEmail() const { return userEmail_; }
    const QString & userSubscription() const { return userSubscription_; }
    const QUrl & userImage() const { return userImage_; }
    bool isActive() const { return isActive_; }
    bool isPlaying() const { return isPlaying_; }
    bool shuffleEnabled() const { return shuffleEnabled_; }
    bool repeatOneEnabled() const { return repeatOneEnabled_; }
    bool repeatAllEnabled() const { return repeatAllEnabled_; }
    const QString & trackName() const { return trackName_; }
    const QString & trackArtist() const { return trackArtist_; }
    const QString & trackAlbum() const { return trackAlbum_; }
    const QUrl & trackAlbumArt() const { return trackAlbumArt_; }
    int trackPosition() const { return trackPosition_; }
    int trackDuration() const { return trackDuration_; }
    const QString & playlistName() const { return playlistName_; }
    const QVariantList & playlists() const { return playlists_; }
    const QString & deviceName() const { return deviceName_; }
    const QString & deviceType() const { return deviceType_; }
    const QVariantList & devices() const { return devices_; }
    const QString & preferredDevice() const { return preferredDevice_; }
    const QVariantList & searchResults() const { return searchResults_; }
    int deviceVolume() const { return deviceVolume_; }

signals:
    void userNameChanged();
    void userEmailChanged();
    void userSubscriptionChanged();
    void userImageChanged();
    void isActiveChanged();
    void isPlayingChanged();
    void shuffleEnabledChanged();
    void repeatOneEnabledChanged();
    void repeatAllEnabledChanged();
    void trackNameChanged();
    void trackArtistChanged();
    void trackAlbumChanged();
    void trackAlbumArtChanged();
    void trackPositionChanged();
    void trackDurationChanged();
    void playlistNameChanged();
    void playlistsChanged();
    void deviceNameChanged();
    void deviceTypeChanged();
    void deviceVolumeChanged();
    void devicesChanged();
    void preferredDeviceChanged();
    void searchResultsChanged();
    void clientIDChanged();
    void clientSecretChanged();
    void refreshTokenChanged();

public slots:
    void refresh() override;
    void refreshDevices();
    void refreshUserProfile();
    void refreshPlaylists();

    void play( const QString & uri = QString() );
    void pause();
    void previous();
    void next();
    void seek( int position );
    void enableShuffle( bool value );
    void enableRepeat( bool value, bool all = true );
    void commandDeviceVolume( int value );
    void search( const QString & query );
    void queue( const QString & uri );
    void transfer( const QString & deviceID );

    QString formatDuration( int value );

private slots:
    void handleNetworkReply( int statusCode, QObject * sender, const QJsonDocument & body );
    void refreshAccessToken();

private:
    QString userName_;
    QString userEmail_;
    QString userSubscription_;
    QUrl userImage_;
    bool isActive_;
    bool isPlaying_;
    bool shuffleEnabled_;
    bool repeatOneEnabled_;
    bool repeatAllEnabled_;
    QString trackName_;
    QString trackArtist_;
    QString trackAlbum_;
    QUrl trackAlbumArt_;
    int trackPosition_;
    int trackDuration_;
    QString playlistName_;
    QVariantList playlists_;
    QString deviceName_;
    QString deviceType_;
    int deviceVolume_;
    QVariantList devices_;
    QString preferredDevice_;
    QString preferredDeviceID_;
    QVariantList searchResults_;
    QString clientID_;
    QString clientSecret_;
    QString refreshToken_;
    QString accessToken_;
    QString accessTokenType_;

    QString market_;
    QByteArray accessTokenAuthorization_;
    QTimer accessTokenRefreshTimer_;
    QTimer inactivityTimer_;
    QTimer actionSubmissionTimer_;

    void sendRequest( const QUrl & destination,
                      QNetworkAccessManager::Operation requestType = QNetworkAccessManager::GetOperation,
                      const QJsonDocument & body = QJsonDocument() );

    Q_DISABLE_COPY_MOVE( VCSpotify )
};

#endif  // VCSPOTIFY_H_
