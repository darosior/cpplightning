#ifndef CPPLIGHTNING_H
#define CPPLIGHTNING_H

#include <jsonrpccpp/client.h>
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>
#include <string>
#include <vector>

class CLightningWrapper
{
private:
    jsonrpc::UnixDomainSocketClient * socketClient;
    jsonrpc::Client * client;
    
public:
    CLightningWrapper(std::string& socket_path);
    ~CLightningWrapper();

    /** Sends a JSON-RPC command to the C-Lightning socket.
     * Used by all following functions to communicate with lightningd.
     */
    Json::Value sendCommand(const std::string& command, const Json::Value& arguments);

    /* Boolean functions are for calls that don't return anything usefull but for which it can be usefull to
     * return a confirmation (true) after execution (some of these commands can take time, like `pay`). */

    /** Set up autoclean of expired invoices. Perform cleanup every `cycleSeconds`, or disable autoclean if 0.
     * Clean up expired invoices that have expired for `expiredBy` seconds. */
    bool autoCleanInvoice(const unsigned int& cycleSeconds=3600, const unsigned int& expiredBy=86400);
    /** Don't run `command`, just verify parameters. */
    bool check(const std::string& command);
    /** Connect to host identified by pubkey@host:port */
    bool connect(const std::string& host);
    /** Delete unpaid invoice `label` with `status` */
    bool delInvoice(const std::string& label, const std::string& status);
    /** Delete all expired invoices that expired as of given `maxExpiryTime` (a UNIX epoch time) */
    bool delExpiredInvoice(const unsigned int& maxExpiryTime);
    /** Delete all expired invoices. */
    bool delExpiredInvoices();
    /** Disconnect from `id` that has previously been connected to using connect; with `force` set, even if it has a current channel */
    bool disconnect(const std::string& id, const bool& force=false);
    /** Send payment specified by bolt11. `msat` should __only__ be specified if not in bolt11. */
    bool pay(const std::string& bolt11, const unsigned int& msat=0, const std::string& label="",
            const unsigned int& riskfactor=10, const float& maxFeePercent=0.5,
            const unsigned int& retryFor=60, const unsigned int& maxDelay=500, const unsigned int exemptFee=5000);
    /** Send peer `id` a ping of length `len` asking for `pongbytes` */
    bool ping(const std::string& id, const unsigned int& len=128, const unsigned int& pongbytes=128);
    /** Shut down the lightningd process */
    bool stop();
    /** Wait for the next invoice to be paid, after `lastpay_index` (if specified) */
    bool waitAnyInvoice(const unsigned int& lastpayIndex=0);
    /** Wait for an incoming payment matching the invoice with `label`, or if the invoice expires */
    bool waitInvoice(const std::string& label);
    
    
    /* String functions are for calls which return a string as the only usefull information (like an address,
     * an invoice, or a txid).*/

    /** Fund channel with `id` using `sats` satoshis, at optional `feerate`.If `announce` is set to false, the channel is considered private.
     * Only use outputs that have `minconf` confirmations. */
    std::string fundChannel(const std::string& id, const unsigned int& sats, const unsigned int& feerate=0,
            const bool& announce=true, const unsigned int& minconf=1);
    /** Get the value of a configuration parameter */
    std::string getConfig(const std::string& param);
    /** Create an invoice for `msats` millisatoshis with `label` and `description` with optional `expiry` seconds (default 1 hour),
     * optional `fallbacks` address list (default empty list) and optional `preimage` (default autogenerated) */
    std::string getInvoice(const unsigned int& msats, const std::string& label, const std::string& description,
            const unsigned int& expiry=3600, const std::vector<std::string>& fallbacks={},
            const std::string& preimage="");
    
    /** Get a new address to fund a channel. Address `type` can be one of {bech32, p2sh-segwit} (default is bech32) */
    std::string newAddr(const std::string& type="bech32");
    /** Send to `address` `sats` satoshis via Bitcoin transaction, at optional `feerate`, using outputs with at least `minconf` confirmations. */
    std::string withdraw(const std::string& address, const unsigned int& sats, const unsigned int& feerate=0,
            const unsigned int& minconf=1);
    

    /* JSON Value functions are for calls whether complex, long, that need the JSON syntax (used in lightningd), or simply
     * not yet implemented.*/

    /** Close the channel with `id` (either peer ID, channel ID, or short channel ID). If `force` (default false) is true,
     * force a unilateral close after `timeout` seconds (default 30), otherwise just schedule a mutual close later and fail after timing out. */
    Json::Value close(const std::string& id, const bool& force=false, const unsigned int& timeout=30);
    /** Decode `bolt11` */
    Json::Value decodepay(const std::string& bolt11);
    /** Return feerate estimates, either satoshi-per-kw ({style} perkw) or satoshi-per-kb ({style} perkb). */
    Json::Value getFeerates(const std::string& style="perkb");
    /** Show information about this node */
    Json::Value getInfo();
    /** Show logs, with optional log `level` (info|unusual|debug|io) */
    Json::Value getLog(const std::string& level="info");
    /** Show route to `id` for `msatoshi`, using `riskfactor` and optional `cltv` (default 9). If specified search from `fromid` otherwise use this node as source.
     * Randomize the route with up to `fuzzpercent` (default 5.0) using `seed` as an arbitrary-size string seed. */
    Json::Value getRoute(const std::string& id, const unsigned int& msats, const unsigned int& riskfactor, 
            const unsigned int& cltv=9, const std::string& fromid="", const float& fuzzPercent=5,
            const std::vector<std::string>& exlude={}, const unsigned int& maxhops=20);
    /** List available calls along with a small description for each*/
    Json::Value help();
    /** Returns a verbose description (man page) of a specified `call` */
    Json::Value helpOn(const std::string& call);
    /** Show channel `short_channel_id` or `source` (or all known channels, if not specified) */
    Json::Value listChannels(const std::string& shortChannelId="", const std::string& source="");
    /** List all configuration options */
    Json::Value listConfigs();
    /** List all forwarded payments and their information */
    Json::Value listForwards();
    /** Show available funds from the internal wallet */
    Json::Value listFunds();
    /** Show invoice `label` (or all, if no `label`) */
    Json::Value listInvoices(const std::string& label="");
    /** Show node `id` (or all, if no `id`), in our local network view */
    Json::Value listNodes();
    /** List result of payment {bolt11}, or all */
    Json::Value listPay(const std::string& bolt11="");
    /** Show outgoing payments */
    Json::Value listPayments(const std::string& bolt11="", const std::string& payment_hash="");
    /** Show current peers, if `level` is set, include logs for `id` */
    Json::Value listPeers(const std::string& id="", const std::string& level="");
    /** Show sendpay, old and current, optionally limiting to `bolt11` or `payment_hash`. */
    Json::Value listSendPays(const std::string& bolt11="", const std::string& payment_hash="");
    /** Detail status of attempts to pay `bolt11`, or all if not bolt11 specified.*/
    Json::Value payStatus(const std::string& bolt11="");
    /** Send along `route` in return for preimage of `payment_hash` */
    Json::Value sendPay(const std::string& route, const std::string& payment_hash, const std::string& label="", 
            const unsigned int& msats=0, const std::string& bolt11="");
};

#endif // CPPLIGHTNING_H
