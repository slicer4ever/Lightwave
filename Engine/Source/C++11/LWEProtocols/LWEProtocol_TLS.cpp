#include "LWEProtocols/LWEProtocol_TLS.h"
#include <iostream>
#include <botan/x509path.h>
#include <botan/data_src.h>
#include <LWCore/LWAllocator.h>
#include <LWPlatform/LWFileStream.h>
#include <LWCore/LWLogger.h>
#include <chrono>

std::vector<std::string> LWETLSPolicy::allowed_key_exchange_methods(void) const {
	std::vector<std::string> v = Botan::TLS::Policy::allowed_key_exchange_methods();
	v.push_back("RSA");
	return v;
}

std::vector<Botan::Certificate_Store*> LWETLS_CredentialsManager::trusted_certificate_authorities(const std::string& type, const std::string& context) {
	std::vector<Botan::Certificate_Store*> v;
	if (type == "tls-server") return v;
	for (auto && cs : m_CertStores) v.push_back(cs.get());
	return v;
}

std::vector<Botan::X509_Certificate> LWETLS_CredentialsManager::cert_chain(const std::vector<std::string>& cert_key_types, const std::string& type, const std::string& context) {
	for (auto &&i : m_Credentials) {
		if (std::find(cert_key_types.begin(), cert_key_types.end(), i.m_Key->algo_name()) == cert_key_types.end()) continue;
		if (context != "" && !i.m_Certs[0].matches_dns_name(context)) continue;
		return i.m_Certs;
	}
	return std::vector<Botan::X509_Certificate>();
}

Botan::Private_Key *LWETLS_CredentialsManager::private_key_for(const Botan::X509_Certificate& cert, const std::string& type, const std::string& context) {
	for (auto &&i : m_Credentials) {
		for (auto &&c : i.m_Certs) {
			if (cert == c) return i.m_Key.get();
		}
	}
	return nullptr;
}

bool LWETLS_CredentialsManager::LoadCertficateAndKey(const LWUTF8Iterator &CertFile, const LWUTF8Iterator &KeyFile, Botan::RandomNumberGenerator &Rng) {
	char CertPath[256];
	char KeyPath[256];
	if (!LWFileStream::ParsePath(CertFile, CertPath, sizeof(CertPath))) return false;
	if (!LWFileStream::ParsePath(KeyFile, KeyPath, sizeof(KeyPath))) return false;
	Certificate_Info cert;
	cert.m_Key.reset(Botan::PKCS8::load_key(KeyPath, Rng));
	Botan::DataSource_Stream in(CertPath);
	while (in.check_available(27)){ //27 ~= header size.
		cert.m_Certs.push_back(Botan::X509_Certificate(in));
	}

	m_Credentials.push_back(cert);
	return true;
}

bool LWETLS_CredentialsManager::LoadDefaultCertificateStores(void) {
	try {
		const std::vector<std::string> Paths = { "/etc/ssl/certs", "/usr/share/ca-certificates" };
		for (auto &&P : Paths) {
			std::shared_ptr<Botan::Certificate_Store> cs(new Botan::Certificate_Store_In_Memory(P));
			m_CertStores.push_back(cs);
		}
	}catch (std::exception &e) {
		LWLogCritical(e.what());
		return false;
	}
	return true;
}

LWETLS_CredentialsManager::LWETLS_CredentialsManager() {
}

void LWETLSCallbacks::tls_emit_data(const uint8_t data[], size_t size) {
	if(!LWLogCriticalIf((bool)m_Socket && m_Socket->IsValid(), "TLS connection was closed, dropping emit data.")) return;
	uint32_t Len = (uint32_t)size;
	uint32_t r = m_Socket->SendAll(data, Len);
	LWLogCriticalIf<256>(r==Len, "Emitting data from '{}' failed: {}", m_Socket->GetLocalAddr(), LWProtocolManager::GetError());
	return;
}

void LWETLSCallbacks::tls_record_received(uint64_t seq_no, const uint8_t data[], size_t size) {
	m_Protocol.ProcessTLSData(m_Socket, (const char*)data, (uint32_t)size);
	return;
}

void LWETLSCallbacks::tls_alert(Botan::TLS::Alert alert) {
	//LWELogCritical<256>("TLS alert: {}", alert.type_string());
	return;
}

bool LWETLSCallbacks::tls_session_established(const Botan::TLS::Session& session) {
	return true;
}

void LWETLSCallbacks::tls_verify_cert_chain(const std::vector<Botan::X509_Certificate>& cert_chain, const std::vector<std::shared_ptr<const Botan::OCSP::Response>>& ocsp_responses, const std::vector<Botan::Certificate_Store *>& trusted_roots, Botan::Usage_Type usage, const std::string& hostname, const Botan::TLS::Policy& policy) {
	if (cert_chain.empty()) return;
	Botan::Path_Validation_Restrictions Restrictions(policy.require_cert_revocation_info(), policy.minimum_signature_strength());
	auto Timeout = std::chrono::milliseconds(1000);
	Botan::Path_Validation_Result Result = Botan::x509_path_validate(cert_chain, Restrictions, trusted_roots, hostname, usage, std::chrono::system_clock::now(), Timeout, ocsp_responses);
	if (Result.successful_validation()) {
		auto status = Result.all_statuses();
		if (status.size() > 0 && status[0].count(Botan::Certificate_Status_Code::OCSP_RESPONSE_GOOD)) {
			//std::cout << "Valid OCSP response for this server!" << std::endl;
		}
	}
}

LWETLSCallbacks &LWETLSCallbacks::SetSocket(LWRef<LWSocket> &Socket) {
	m_Socket = Socket;
	return *this;
}

LWETLSCallbacks &LWETLSCallbacks::SetClient(Botan::TLS::Client *Cli) {
	m_Client = Cli;
	return *this;
}

LWETLSCallbacks &LWETLSCallbacks::SetServer(Botan::TLS::Server *Srv) {
	m_Server = Srv;
	return *this;
}

Botan::TLS::Client *LWETLSCallbacks::GetClient(void) {
	return m_Client;
}

Botan::TLS::Server *LWETLSCallbacks::GetServer(void) {
	return m_Server;
}

LWETLSCallbacks::LWETLSCallbacks(LWRef<LWSocket> &Sock, LWEProtocol_TLS &Protocol) : m_Socket(Sock), m_Protocol(Protocol) {}

LWETLSCallbacks::~LWETLSCallbacks() {
	LWAllocator::Destroy(m_Client);
	LWAllocator::Destroy(m_Server);
}

LWProtocol &LWEProtocol_TLS::Read(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	char Buffer[1024 * 64]; //64KB read buffer.
	int32_t r = Socket->Receive(Buffer, sizeof(Buffer));

	if (r <= 0) {
		LWLogCriticalIf<256>(r!=-1, "Socket '{}' Error: {}", Socket->GetRemoteAddr(), LWProtocolManager::GetError());
		Socket->MarkClosable();
		return *this;
	}
	LWRef<LWETLSCallbacks> CB = GetSocketDataFor(Socket, m_CallbackMap, m_CallbackMapMutex);

	Botan::TLS::Client *TLSCli = nullptr;
	Botan::TLS::Server *TLSSrv = nullptr;
	if (!CB) {
		CB = SetSocketDataFor(Socket, m_Allocator.CreateRef<LWETLSCallbacks>(Socket, *this), m_CallbackMap, m_CallbackMapMutex);
		CB->SetServer(m_Allocator.Create<Botan::TLS::Server>(*CB, *m_SessionManager, m_CredentialsManager, m_Policy, m_RNG));
	}
	TLSCli = CB->GetClient();
	TLSSrv = CB->GetServer();
	if(TLSCli && TLSCli->is_closed()){
		Socket->MarkClosable();
		return *this;
	}
	if (TLSSrv && TLSSrv->is_closed()) {
		Socket->MarkClosable();
		return *this;
	}
	if (TLSCli) TLSCli->received_data((uint8_t*)Buffer, r);
	if (TLSSrv) TLSSrv->received_data((uint8_t*)Buffer, r);
	return *this;
}

LWProtocol &LWEProtocol_TLS::SocketClosed(LWRef<LWSocket> &Socket, LWProtocolManager &Manager) {
	ExchangeSocketsData(Socket, LWRef<LWETLSCallbacks>(), m_CallbackMap, m_CallbackMapMutex);
	return *this;
}

LWProtocol &LWEProtocol_TLS::ProcessTLSData(LWRef<LWSocket> &Socket, const void *Data, uint32_t DataLen) {
	return *this;
}

uint32_t LWEProtocol_TLS::Send(LWRef<LWSocket> &Socket, const void *Buffer, uint32_t Len) {
	LWRef<LWETLSCallbacks> CB = GetSocketDataFor(Socket, m_CallbackMap, m_CallbackMapMutex);
	Botan::TLS::Client *TLSCli = nullptr;
	Botan::TLS::Server *TLSSrv = nullptr;
	if (!CB) {
		CB = SetSocketDataFor(Socket, m_Allocator.CreateRef<LWETLSCallbacks>(Socket, *this), m_CallbackMap, m_CallbackMapMutex);
		CB->SetClient(m_Allocator.Create<Botan::TLS::Client>(*CB, *m_SessionManager, m_CredentialsManager, m_Policy, m_RNG, Botan::TLS::Server_Information(), Botan::TLS::Protocol_Version::latest_tls_version()));
	}
	TLSCli = CB->GetClient();
	TLSSrv = CB->GetServer();
	if (TLSCli) {
		if (TLSCli->is_closed()) return -1;
		if (!TLSCli->is_active()) return 0;
		TLSCli->send((uint8_t*)Buffer, Len);
	}else if (TLSSrv) {
		if (TLSSrv->is_closed()) return -1;
		if (!TLSSrv->is_active()) return 0;
		TLSSrv->send((uint8_t*)Buffer, Len);
	}
	return Len;
}

LWEProtocol_TLS::LWEProtocol_TLS(uint32_t ProtocolID, LWAllocator &Allocator, const LWUTF8Iterator &CertFile, const LWUTF8Iterator &KeyFile) : LWProtocol(ProtocolID), m_Allocator(Allocator) {
	m_SessionManager = Allocator.Create<Botan::TLS::Session_Manager_In_Memory>(m_RNG);
	if (CertFile.isInitialized() && KeyFile.isInitialized()) m_CredentialsManager.LoadCertficateAndKey(CertFile, KeyFile, m_RNG);
	else m_CredentialsManager.LoadDefaultCertificateStores();
}


LWEProtocol_TLS::~LWEProtocol_TLS() {
	LWAllocator::Destroy(m_SessionManager);
}