#ifndef LWEPROTOCOLTLS_H
#define LWEPROTOCOLTLS_H
#include <LWNetwork/LWProtocol.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWNetwork/LWSocket.h>
#include <botan/pkcs8.h>
#include <botan/credentials_manager.h>
#include <botan/x509self.h>
#include <botan/x509cert.h>
#include <botan/tls_client.h>
#include <botan/tls_server.h>
#include <botan/auto_rng.h>
#include <vector>
#include <string>

class LWEProtocolTLS;

class LWETLSPolicy : public Botan::TLS::Policy {
public:
	virtual std::vector<std::string> allowed_key_exchange_methods() const;

};

class LWETLS_CredentialsManager : public Botan::Credentials_Manager {
public:
	std::vector<Botan::Certificate_Store*> trusted_certificate_authorities(const std::string& type, const std::string& context);

	std::vector<Botan::X509_Certificate> cert_chain(const std::vector<std::string>& cert_key_types, const std::string& type, const std::string& context);

	Botan::Private_Key *private_key_for(const Botan::X509_Certificate& cert, const std::string& type, const std::string& context);

	bool LoadCertficateAndKey(const LWText &CertFile, const LWText &KeyFile, Botan::RandomNumberGenerator &Rng);

	bool LoadDefaultCertificateStores(void);

	LWETLS_CredentialsManager();
private:
	struct Certificate_Info {
		std::vector<Botan::X509_Certificate> m_Certs;
		std::shared_ptr<Botan::Private_Key> m_Key;
	};
	std::vector<Certificate_Info> m_Credentials;
	std::vector<std::shared_ptr<Botan::Certificate_Store>> m_CertStores;
};

class LWETLSCallbacks : public Botan::TLS::Callbacks {
public:
	void tls_emit_data(const uint8_t data[], size_t size);

	void tls_record_received(uint64_t seq_no, const uint8_t data[], size_t size);

	void tls_alert(Botan::TLS::Alert alert);

	void tls_verify_cert_chain(const std::vector<Botan::X509_Certificate> &cert_chain, const std::vector < std::shared_ptr<const Botan::OCSP::Response>> &ocsp_responses, const std::vector<Botan::Certificate_Store*> &trusted_roots, Botan::Usage_Type usage, const std::string &Hostname, const Botan::TLS::Policy &Policy);

	bool tls_session_established(const Botan::TLS::Session &session);

	LWETLSCallbacks &SetClient(Botan::TLS::Client *Cli);

	LWETLSCallbacks &SetServer(Botan::TLS::Server *Srv);

	LWETLSCallbacks &SetSocket(LWSocket *Socket);

	Botan::TLS::Client *GetClient(void);

	Botan::TLS::Server *GetServer(void);

	LWETLSCallbacks(LWSocket *Sock, LWEProtocolTLS &Protocol);

	~LWETLSCallbacks();
private:
	Botan::TLS::Client *m_Client;
	Botan::TLS::Server *m_Server;
	LWEProtocolTLS &m_Protocol;
	LWSocket *m_Socket;
};

class LWEProtocolTLS : public LWProtocol {
public:

	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &ProcessTLSData(LWSocket &Socket, const char *Data, uint32_t DataLen);

	uint32_t Send(LWSocket &Socket, const char *Buffer, uint32_t Len);
	
	LWEProtocolTLS(uint32_t ProtocolID, LWAllocator &Allocator, const char *CertFile = nullptr, const char *KeyFile = nullptr);

	~LWEProtocolTLS();
protected:
	LWAllocator &m_Allocator;
	Botan::AutoSeeded_RNG m_RNG;
	LWETLSPolicy m_Policy;
	Botan::TLS::Session_Manager_In_Memory *m_SessionManager;
	LWETLS_CredentialsManager m_CredentialsManager;
	uint32_t m_ProtocolID;

};

#endif