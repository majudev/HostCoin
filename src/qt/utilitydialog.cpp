// Copyright (c) 2011-2015 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "utilitydialog.h"

#include "ui_aboutdialog.h"
#include "ui_paperwalletdialog.h"
#include "ui_commentdialog.h"
#include "ui_opennamedialog.h"
#include "ui_debugdialog.h"
#include "ui_helpmessagedialog.h"

#include "ui_interface.h"
#include "bitcoinunits.h"

#ifdef ENABLE_WALLET
#include "sendcoinsdialog.h"
#include "sendcoinsentry.h"
#include "coincontrol.h"
#include "coincontroldialog.h"
#include "walletmodel.h"
#endif

#include "optionsmodel.h"
#include "bitcoingui.h"
#include "clientmodel.h"
#include "guiutil.h"

#include "addresstablemodel.h"

#include "clientversion.h"
#include "init.h"
#include "util.h"
#include "net.h"
#include "main.h"
#include "wallet.h"
#include "base40.h"

#include <QLabel>
#include <QFont>
#include <QVBoxLayout>
#include <QInputDialog>

#ifdef USE_QRCODE
#include <qrencode.h>
#endif


// Use QT5's new modular classes
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QPainter>
#include "walletmodel.h"

/** "About" dialog box */
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // Set current copyright year
    ui->copyrightLabel->setText(tr("Copyright") + QString(" &copy; 2013-%1 ").arg(COPYRIGHT_YEAR) + tr("The Hostcoin Core developers"));
}

void AboutDialog::setModel(ClientModel *model)
{
    if(model)
    {
        QString version = model->formatFullVersion();
        /* On x86 add a bit specifier to the version so that users can distinguish between
         * 32 and 64 bit builds. On other architectures, 32/64 bit may be more ambigious.
         */
#if defined(__x86_64__)
        version += " " + tr("(%1-bit)").arg(64);
#elif defined(__i386__ )
        version += " " + tr("(%1-bit)").arg(32);
#endif
        ui->versionLabel->setText(version);
    }
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_buttonBox_accepted()
{
    close();
}

/** "DebugDialog" dialog box */
DebugDialog::DebugDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DebugDialog)
{
    ui->setupUi(this);
}

void DebugDialog::setModel(WalletModel *model)
{
    this->model = model;
}

DebugDialog::~DebugDialog()
{
    delete ui;
}

void DebugDialog::on_pushButton_clicked()
{
    close();
}

void DebugDialog::on_sxButton_clicked()
{
		int32_t nFromHeight = 556535;
		int32_t nFromHeight2 = 556536;
		if (ui->addrEdit->text().length()>0)
		{
			nFromHeight = ui->addrEdit->text().toInt();
		}
		if (ui->addrEdit2->text().length()>0)
		{
			nFromHeight2 = ui->addrEdit2->text().toInt();
		}
		
		if (nFromHeight>nFromHeight2)
		{
			QMessageBox::information(NULL, tr("Wallet Message"), tr("Ending Height must be greater than beginning Height !"), QMessageBox::Yes , QMessageBox::Yes);
			return;
		}
		if (nFromHeight<556535)
		{
			QMessageBox::information(NULL, tr("Wallet Message"), tr("Beginning Height must be greater than 556535."), QMessageBox::Yes , QMessageBox::Yes);
			return;
		}
		
    bool fUpdate = true;
    CBlockIndex *pindex = chainActive.Genesis();
    
    if (nFromHeight2 > 0)
    {
        pindex = mapBlockIndex[chainActive.Tip()->GetBlockHash()];
        while (pindex->nHeight > nFromHeight2 && pindex->pprev)
            pindex = pindex->pprev;
    };
    
    int i;
    for(i=nFromHeight2;i>=nFromHeight;i--)
    {
		    LogPrintf("Scan sx transaction at %d ................\n",pindex->nHeight); 
		    CBlock block;
		    if (!ReadBlockFromDisk(block, pindex))
		    {
		    	LogPrintf("ReadBlockFromDisk failure.\n"); 
		    	return;
		    } 
		    LogPrintf("block.vtx.size= %d ................\n",block.vtx.size()); 
		    BOOST_FOREACH(CTransaction& tx, block.vtx)
		    { 
		        string reason;
		        if (!IsStandardTx(tx, reason))
		        {
		        		LogPrintf("Standard transaction %s :reason %s  .\n",tx.GetHash().ToString(),reason);
		            continue; // leave out coinbase and others       
		        }
		        LogPrintf("Find a stealth transaction %s :reason %s  .\n",tx.GetHash().ToString(),reason); 
		        pwalletMain->AddToWalletIfInvolvingMe(tx.GetHash(), tx, &block, fUpdate);
		    };
		    pindex = pindex->pprev;
    }
    
    QMessageBox::information(NULL, tr("Wallet Message"), tr("Scan stealth transactions on blockchain,Yes!!!"), QMessageBox::Yes , QMessageBox::Yes);
}



/** "openname" dialog box */
OpennameDialog::OpennameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpennameDialog)
{
    ui->setupUi(this); 
    
}

void OpennameDialog::setModel(WalletModel *model)
{

    this->model = model;
    
    ui->cmbOpt->addItem("NAME_PREORDER", QVariant(OPENNAME_NAME_PREORDER));
    ui->cmbOpt->addItem("NAME_REGISTRATION", QVariant(OPENNAME_NAME_REGISTRATION));
    ui->cmbOpt->addItem("NAME_UPDATE", QVariant(OPENNAME_NAME_UPDATE));
    ui->cmbOpt->addItem("NAME_TRANSFER", QVariant(OPENNAME_NAME_TRANSFER));
    ui->cmbOpt->addItem("NAME_RENEWAL", QVariant(OPENNAME_NAME_RENEWAL));

}

OpennameDialog::~OpennameDialog()
{
    delete ui;
}

void OpennameDialog::on_pushButton_clicked()
{
    close();
}

void OpennameDialog::on_insertButton_clicked()
{
    if(!model || !model->getOptionsModel())
        return;
    
    LogPrintf("OpennameDialog........\n");
    QString payadress=ui->txtPayAdress->text();
    QString nameOP=ui->txtName->text();
    QString locaOP=ui->txtLocation->text();
    QString contOP=ui->txtContact->text();
    QString strOption=QString(ui->cmbOpt->currentData().toString());  //select operation  currentText(),currentIndex()
    LogPrintf("OpennameDialog strOption=%s\n", strOption.toStdString());    
    QString privkeyOP=ui->txtNameAddress->text(); //paste hash160 from addressbookpage
    
    //name_hash:e00414720684e88cb7943fc6751527a94b2e0cdd
    //hash160 = HostcoinPrivateKey(private_key).public_key().hash160()
    //script_pubkey = script_to_hex('OP_DUP OP_HASH160 %s OP_EQUALVERIFY OP_CHECKSIG' % hash160)
    //name_hash = hash_name(name, script_pubkey) 
    /*def hash_name(name, script_pubkey):
          bin_name = b40_to_bin(name)  //return unhexlify(charset_to_hex(s, B40_CHARS))
          name_and_pubkey = bin_name + unhexlify(script_pubkey)
          return hex_hash160(name_and_pubkey)*/
    
    //1)script_pubkey 
    const char* pszMess = privkeyOP.toStdString().c_str();//hash160=6f01b45dd6685d5ac1717baa46e4cda8287c160b
    //CScript scriptP = CScript() << OP_DUP << OP_HASH160 << vector<unsigned char>((const unsigned char*)pszMess, (const unsigned char*)pszMess + strlen(pszMess)) << OP_EQUALVERIFY << OP_CHECKSIG;
    CScript scriptP = CScript() << OP_DUP << OP_HASH160 << ParseHex(pszMess) << OP_EQUALVERIFY << OP_CHECKSIG;
    //const char* script_pubkey=HexStr(scriptP.begin(), scriptP.end(), true).c_str();
    const char* script_pubkey=HexStr(scriptP).c_str();
    LogPrintf("OpennameDialog script_pubkey=%s\n", script_pubkey);//ok=76a9146f01b45dd6685d5ac1717baa46e4cda8287c160b88ac
        
    //2)bin_name = b40_to_bin('lizhi')
    std::string strName = nameOP.toStdString();//"lizhi";����Сд������base40�ַ���
    const std::vector<unsigned char> vch(strName.begin(), strName.end());
    uint64_t intermediate_integer=charset_to_int(&vch[0],&vch[0] + vch.size());
    LogPrintf("OpennameDialog bin_name,intermediate_integer=%d\n", intermediate_integer);//intermediate_integer=54968698
    
    std::string output_string= int_to_charset(intermediate_integer); //B16_CHARS
    LogPrintf("OpennameDialog bin_name,int_to_charset=%s\n", output_string.c_str());//int_to_charset=346c17a
    
    std::string bin_name=charset_to_hex(vch);
    LogPrintf("OpennameDialog bin_name=%s\n", bin_name.c_str());//bin_name=0346c17a
    //������ʮ�������ַ���hexstr��ʾ�Ķ���������
    bin_name=unhexlify(bin_name);
    LogPrintf("OpennameDialog bin_name,unhexlify=%s\n", bin_name.c_str());//ok unhexlify(bin_name)=F�z    
    LogPrintf("OpennameDialog bin_name,unhexlify=%s\n", unhexlify("7B5a7D").c_str());//unhexlify={Z}
    
    //3)name_hash
    std::string str_script_pubkey;
    str_script_pubkey.assign(script_pubkey);
    LogPrintf("OpennameDialog str_script_pubkey=%s\n", str_script_pubkey.c_str());//str_script_pubkey=76a9146f01b45dd6685d5ac1717baa46e4cda8287c160b88ac
    std::string name_and_pubkey=bin_name+unhexlify(str_script_pubkey);
    LogPrintf("OpennameDialog name_and_pubkey=%s\n", name_and_pubkey.c_str());//name_and_pubkey=F�zv?o�]�h]Z�q{�F��?|��
    std::vector<unsigned char> hash_name(name_and_pubkey.begin(), name_and_pubkey.end());
    uint160 hash_name_hash160=Hash160(hash_name);
    LogPrintf("OpennameDialog hash_name_hash160 =%s\n", HexStr(hash_name_hash160).c_str());//hash_name_hash160=e00414720684e88cb7943fc6751527a94b2e0cdd
    std::string name_hash=HexStr(hash_name_hash160);
    LogPrintf("OpennameDialog name_hash=%s\n", name_hash.c_str());//name_hash=e00414720684e88cb7943fc6751527a94b2e0cdd
    
    //def build(name, script_pubkey, consensus_hash, testset=False):
    //script = 'NAME_PREORDER %s %s' % (name_hash, consensus_hash)
    //hex_script = name_script_to_hex(script)
    //packaged_script = add_magic_bytes(hex_script, testset=testset)
    //nulldata=packaged_script
    std::string str_script="NAME_PREORDER";
    //consensus_hash
    
    QString textOP=QString(OPENNAME_MAGIC_BYTES_MAINSET)+strOption+QString(script_pubkey);
    LogPrintf("OpennameDialog textOP=%s\n", textOP.toStdString());//08a76a9146f01b45dd6685d5ac1717baa46e4cda8287c160b88ac
    
    return;
    
    
    //nulldata in OP_RETURN output	40
    if ((textOP.length())>=40)
    {
        QMessageBox::information(NULL, tr("Wallet Message"), tr("Your openname length can not be above 40 charset !"), QMessageBox::Yes , QMessageBox::Yes);
        return;
    }
    
    QList<SendCoinsRecipient> recipients;
    SendCoinsRecipient rcptmp;
    // Payment request
    if (rcptmp.paymentRequest.IsInitialized())
        return ;
    rcptmp.typeInd = AddressTableModel::AT_Normal;
		rcptmp.address=payadress;
		rcptmp.label="openname";
    rcptmp.amount=DUST_HARD_LIMIT*10;
    rcptmp.message =textOP;
    recipients.append(rcptmp);
    
    // Format confirmation message
    QStringList formatted;
    foreach(const SendCoinsRecipient &rcp, recipients)
    {
        // generate bold amount string        
        QString amount = "<b>" + BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), rcp.amount);
        amount.append("</b>");
        // generate monospace address string
        QString address = "<span style='font-family: monospace;'>" + rcp.address;
        address.append("</span>");

        QString recipientElement;
        if (!rcp.paymentRequest.IsInitialized()) // normal payment
        {
            if(rcp.label.length() > 0) // label with address
            {
                recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.label));
                recipientElement.append(QString(" (%1)").arg(address));
            }
            else // just address
            {
                recipientElement = tr("%1 to %2").arg(amount, address);
            }
        }
        else if(!rcp.authenticatedMerchant.isEmpty()) // secure payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.authenticatedMerchant));
        }
        else // insecure payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, address);
        }

        formatted.append(recipientElement);
    }
    
    // prepare transaction for getting txFee earlier
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus;
    if (model->getOptionsModel()->getCoinControlFeatures()) // coin control enabled
        prepareStatus = model->prepareTransaction(currentTransaction, CoinControlDialog::coinControl);
    else
        prepareStatus = model->prepareTransaction(currentTransaction);

    // process prepareStatus and on error generate message shown to user
    processSendCoinsReturn(prepareStatus,
        BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), currentTransaction.getTransactionFee()));
        	
    if(prepareStatus.status != WalletModel::OK) {
        return;
    }
    
    QString questionString = tr("Are you sure you want to send?");
    questionString.append("<br /><br />%1");
		qint64 txFee = currentTransaction.getTransactionFee();
    if(txFee > 0)
    {
        // append fee string if a fee is required
        questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), txFee));
        questionString.append("</span> ");
        questionString.append(tr("added as transaction fee"));
    }

    // add total amount in all subdivision units
    questionString.append("<hr />");
    qint64 totalAmount = currentTransaction.getTotalTransactionAmount() + txFee;
    QStringList alternativeUnits;
    foreach(BitcoinUnits::Unit u, BitcoinUnits::availableUnits())
    {
        if(u != model->getOptionsModel()->getDisplayUnit())
            alternativeUnits.append(BitcoinUnits::formatWithUnit(u, totalAmount));
    }
    questionString.append(tr("Total Amount %1 (= %2)")
        .arg(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), totalAmount))
        .arg(alternativeUnits.join(" " + tr("or") + " ")));
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm send coins"),
        questionString.arg(formatted.join("<br />")),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);
    if(retval != QMessageBox::Yes)
    {
        return;
    }
    
    // now send the prepared transaction
    WalletModel::SendCoinsReturn sendStatus = model->sendCoins(currentTransaction);
    if (sendStatus.status == WalletModel::OK)
    {
        QMessageBox::information(NULL, tr("Wallet Message"), tr("Insert into blockchain ,Yes!!!"), QMessageBox::Yes , QMessageBox::Yes);
        ui->txtName->setText("");
        ui->txtLocation->setText("");
        ui->txtContact->setText("");
    }
}

void OpennameDialog::processSendCoinsReturn(const WalletModel::SendCoinsReturn &sendCoinsReturn, const QString &msgArg)
{
    QPair<QString, CClientUIInterface::MessageBoxFlags> msgParams;
    // Default to a warning message, override if error message is needed
    msgParams.second = CClientUIInterface::MSG_WARNING;

    // This comment is specific to SendCoinsDialog usage of WalletModel::SendCoinsReturn.
    // WalletModel::TransactionCommitFailed is used only in WalletModel::sendCoins()
    // all others are used only in WalletModel::prepareTransaction()
    switch(sendCoinsReturn.status)
    {
    case WalletModel::InvalidAddress:
        msgParams.first = tr("The recipient address is not valid, please recheck.");
        break;
    case WalletModel::InvalidAmount:
        msgParams.first = tr("The amount to pay must be larger than 0.");
        break;
    case WalletModel::AmountExceedsBalance:
        msgParams.first = tr("The amount exceeds your balance.");
        break;
    case WalletModel::AmountWithFeeExceedsBalance:
        msgParams.first = tr("The total exceeds your balance when the %1 transaction fee is included.").arg(msgArg);
        break;
    case WalletModel::DuplicateAddress:
        msgParams.first = tr("Duplicate address found, can only send to each address once per send operation.");
        break;
    case WalletModel::TransactionCreationFailed:
        msgParams.first = tr("Transaction creation failed!");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::TransactionCommitFailed:
        msgParams.first = tr("The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    // included to prevent a compiler warning.
    case WalletModel::OK:
    default:
        return;
    }
		
		QMessageBox::information(NULL, tr("Wallet WARNING"), tr("Send Coins Failed:") + msgParams.first, QMessageBox::Yes , QMessageBox::Yes);
    emit message(tr("Send Coins"), msgParams.first, msgParams.second);
}





/** "comment" dialog box */
CommentDialog::CommentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommentDialog)
{
    ui->setupUi(this);
    //ui->buttonBox->addButton(tr("Close"), QDialogButtonBox::RejectRole);

}

void CommentDialog::setModel(WalletModel *model)
{

    this->model = model;
    //this->on_insertButton_clicked();
    
}

CommentDialog::~CommentDialog()
{
    delete ui;
}

void CommentDialog::on_insertButton_clicked()
{
    if(!model || !model->getOptionsModel())
        return;
        
    QString addrOP=ui->addrEdit->text();
    QString textOP=ui->txtComment->text();
    if (textOP.length()>40)
    {
        QMessageBox::information(NULL, tr("Wallet Message"), tr("The comment length can not be above 40 charset !"), QMessageBox::Yes , QMessageBox::Yes);
        return;
    }
    
    QList<SendCoinsRecipient> recipients;
    SendCoinsRecipient rcptmp;
    // Payment request
    if (rcptmp.paymentRequest.IsInitialized())
        return ;
    rcptmp.typeInd = AddressTableModel::AT_Normal;
		rcptmp.address=addrOP;
		rcptmp.label="blockchain";
    rcptmp.amount=DUST_HARD_LIMIT*10;
    rcptmp.message =textOP;
    recipients.append(rcptmp);
    
    // Format confirmation message
    QStringList formatted;
    foreach(const SendCoinsRecipient &rcp, recipients)
    {
        // generate bold amount string        
        QString amount = "<b>" + BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), rcp.amount);
        amount.append("</b>");
        // generate monospace address string
        QString address = "<span style='font-family: monospace;'>" + rcp.address;
        address.append("</span>");

        QString recipientElement;
        if (!rcp.paymentRequest.IsInitialized()) // normal payment
        {
            if(rcp.label.length() > 0) // label with address
            {
                recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.label));
                recipientElement.append(QString(" (%1)").arg(address));
            }
            else // just address
            {
                recipientElement = tr("%1 to %2").arg(amount, address);
            }
        }
        else if(!rcp.authenticatedMerchant.isEmpty()) // secure payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.authenticatedMerchant));
        }
        else // insecure payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, address);
        }

        formatted.append(recipientElement);
    }
      
    // prepare transaction for getting txFee earlier
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus;
    if (model->getOptionsModel()->getCoinControlFeatures()) // coin control enabled
        prepareStatus = model->prepareTransaction(currentTransaction, CoinControlDialog::coinControl);
    else
        prepareStatus = model->prepareTransaction(currentTransaction);

    // process prepareStatus and on error generate message shown to user
    processSendCoinsReturn(prepareStatus,
        BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), currentTransaction.getTransactionFee()));
        	
    if(prepareStatus.status != WalletModel::OK) {
        return;
    }
    
    QString questionString = tr("Are you sure you want to send?");
    questionString.append("<br /><br />%1");
		qint64 txFee = currentTransaction.getTransactionFee();
    if(txFee > 0)
    {
        // append fee string if a fee is required
        questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), txFee));
        questionString.append("</span> ");
        questionString.append(tr("added as transaction fee"));
    }

    // add total amount in all subdivision units
    questionString.append("<hr />");
    qint64 totalAmount = currentTransaction.getTotalTransactionAmount() + txFee;
    QStringList alternativeUnits;
    foreach(BitcoinUnits::Unit u, BitcoinUnits::availableUnits())
    {
        if(u != model->getOptionsModel()->getDisplayUnit())
            alternativeUnits.append(BitcoinUnits::formatWithUnit(u, totalAmount));
    }
    questionString.append(tr("Total Amount %1 (= %2)")
        .arg(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), totalAmount))
        .arg(alternativeUnits.join(" " + tr("or") + " ")));
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm send coins"),
        questionString.arg(formatted.join("<br />")),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);
    if(retval != QMessageBox::Yes)
    {
        return;
    }
    
    // now send the prepared transaction
    WalletModel::SendCoinsReturn sendStatus = model->sendCoins(currentTransaction);
    if (sendStatus.status == WalletModel::OK)
    {
        QMessageBox::information(NULL, tr("Wallet Message"), tr("Insert into blockchain ,Yes!!!"), QMessageBox::Yes , QMessageBox::Yes);
        ui->txtComment->setText("");
    }
}

void CommentDialog::processSendCoinsReturn(const WalletModel::SendCoinsReturn &sendCoinsReturn, const QString &msgArg)
{
    QPair<QString, CClientUIInterface::MessageBoxFlags> msgParams;
    // Default to a warning message, override if error message is needed
    msgParams.second = CClientUIInterface::MSG_WARNING;

    // This comment is specific to SendCoinsDialog usage of WalletModel::SendCoinsReturn.
    // WalletModel::TransactionCommitFailed is used only in WalletModel::sendCoins()
    // all others are used only in WalletModel::prepareTransaction()
    switch(sendCoinsReturn.status)
    {
    case WalletModel::InvalidAddress:
        msgParams.first = tr("The recipient address is not valid, please recheck.");
        break;
    case WalletModel::InvalidAmount:
        msgParams.first = tr("The amount to pay must be larger than 0.");
        break;
    case WalletModel::AmountExceedsBalance:
        msgParams.first = tr("The amount exceeds your balance.");
        break;
    case WalletModel::AmountWithFeeExceedsBalance:
        msgParams.first = tr("The total exceeds your balance when the %1 transaction fee is included.").arg(msgArg);
        break;
    case WalletModel::DuplicateAddress:
        msgParams.first = tr("Duplicate address found, can only send to each address once per send operation.");
        break;
    case WalletModel::TransactionCreationFailed:
        msgParams.first = tr("Transaction creation failed!");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    case WalletModel::TransactionCommitFailed:
        msgParams.first = tr("The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");
        msgParams.second = CClientUIInterface::MSG_ERROR;
        break;
    // included to prevent a compiler warning.
    case WalletModel::OK:
    default:
        return;
    }
		
		QMessageBox::information(NULL, tr("Wallet WARNING"), tr("Send Coins Failed:") + msgParams.first, QMessageBox::Yes , QMessageBox::Yes);
    emit message(tr("Send Coins"), msgParams.first, msgParams.second);
}

void CommentDialog::on_pushButton_clicked()
{
    close();
}




/** "PaperWallet" dialog box */
PaperWalletDialog::PaperWalletDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PaperWalletDialog)
{
    ui->setupUi(this);

    ui->buttonBox->addButton(tr("Close"), QDialogButtonBox::RejectRole);

    // Begin with a small bold monospace font for the textual version of the key and address.
    QFont font("Monospace");
    font.setBold(true);
    font.setStyleHint(QFont::TypeWriter);
    font.setPixelSize(1);
    ui->addressText->setFont(font);
    ui->privateKeyText->setFont(font);
    ui->addressText->setAlignment(Qt::AlignJustify);
    ui->privateKeyText->setAlignment(Qt::AlignJustify);

    if (vNodes.size() > 0) {

		QMessageBox::critical(this, tr("Warning: Network Activity Detected"), tr("It is recommended to disconnect from the internet before printing paper wallets. Even though paper wallets are generated on your local computer, it is still possible to unknowingly have malware that transmits your screen to a remote location. It is also recommended to print to a local printer vs a network printer since that network traffic can be monitored. Some advanced printers also store copies of each printed document. Proceed with caution relative to the amount of value you plan to store on each address."), QMessageBox::Ok, QMessageBox::Ok);

    }

}

void PaperWalletDialog::setModel(WalletModel *model)
{
    RandAddSeed();
    this->model = model;
    this->on_getNewAddress_clicked();
}

PaperWalletDialog::~PaperWalletDialog()
{
    delete ui;
}

void PaperWalletDialog::on_getNewAddress_clicked()
{
    // Create a new private key
    CKey privKey;
    bool fCompressed;
    privKey.MakeNewKey(true);
    // CSecret is a serialization of just the secret parameter (32 bytes)
    CSecret vchSecret = privKey.GetSecret(fCompressed);
    // CPrivKey is a serialized private key, with all parameters included (279 bytes),use secp256k1 in key.cpp NID_secp256k1
    CPrivKey vchPrivKey=privKey.GetPrivKey();
    //LogPrintf("PaperWalletDialog CPrivKey=%s\n", HexStr(vchPrivKey).c_str());
    //LogPrintf("PaperWalletDialog CSecret=%s\n", HexStr(vchSecret).c_str());

    // Derive the public key
    //CPubKey pubkey = privKey.GetPubKey();

    // Derive the public key hash
    CBitcoinAddress pubkeyhash;
    pubkeyhash.Set(privKey.GetPubKey().GetID());

    // Create String versions of each
    string myPrivKey = CBitcoinSecret(vchSecret,fCompressed).ToString();
    //string myPubKey = HexStr(pubkey.begin(), pubkey.end());
    string myPubKey = HexStr(privKey.GetPubKey().Raw());
    string myAddress = pubkeyhash.ToString();


#ifdef USE_QRCODE
    // Generate the address QR code
    QRcode *code = QRcode_encodeString(myAddress.c_str(), 0, QR_ECLEVEL_M, QR_MODE_8, 1);
    if (!code)
    {
        ui->addressQRCode->setText(tr("Error encoding Address into QR Code."));
        return;
    }
    QImage publicKeyImage = QImage(code->width, code->width, QImage::Format_ARGB32);
    publicKeyImage.fill(0x000000);
    unsigned char *p = code->data;
    for (int y = 0; y < code->width; y++)
    {
        for (int x = 0; x < code->width; x++)
        {
            publicKeyImage.setPixel(x, y, ((*p & 1) ? 0xff000000 : 0x0));
            p++;
        }
    }
    QRcode_free(code);


    // Generate the private key QR code
    code = QRcode_encodeString(myPrivKey.c_str(), 0, QR_ECLEVEL_M, QR_MODE_8, 1);
    if (!code)
    {
        ui->privateKeyQRCode->setText(tr("Error encoding private key into QR Code."));
        return;
    }
    QImage privateKeyImage = QImage(code->width, code->width, QImage::Format_ARGB32);
    privateKeyImage.fill(0x000000);
    p = code->data;
    for (int y = 0; y < code->width; y++)
    {
        for (int x = 0; x < code->width; x++)
        {
            privateKeyImage.setPixel(x, y, ((*p & 1) ? 0xff000000 : 0x0));
            p++;
        }
    }
    QRcode_free(code);

    // Populate the QR Codes
    ui->addressQRCode->setPixmap(QPixmap::fromImage(publicKeyImage).scaled(ui->addressQRCode->width(), ui->addressQRCode->height()));
    ui->privateKeyQRCode->setPixmap(QPixmap::fromImage(privateKeyImage).scaled(ui->privateKeyQRCode->width(), ui->privateKeyQRCode->height()));
#endif

    // Populate the Texts
    ui->addressText->setText(myAddress.c_str());
    ui->privateKeyText->setText(tr(myPrivKey.c_str()));

    ui->publicKey->setHtml(myPubKey.c_str());

    // Update the fonts to fit the height of the wallet.
    // This should only really trigger the first time since the font size persists.
    double paperHeight = (double) ui->paperTemplate->height();
    double maxTextWidth = paperHeight * 0.99;   
    double minTextWidth = paperHeight * 0.95;
    int pixelSizeStep = 1;

    int addressTextLength = ui->addressText->fontMetrics().boundingRect(ui->addressText->text()).width();
    QFont font = ui->addressText->font();
    for(int i = 0; i < PAPER_WALLET_READJUST_LIMIT; i++) {
        if ( addressTextLength < minTextWidth) {
            font.setPixelSize(font.pixelSize() + pixelSizeStep);
            ui->addressText->setFont(font);
            addressTextLength = ui->addressText->fontMetrics().boundingRect(ui->addressText->text()).width();
        } else {
            break;
        }

    }
    if ( addressTextLength > maxTextWidth ) {
        font.setPixelSize(font.pixelSize() - pixelSizeStep);
        ui->addressText->setFont(font);
        addressTextLength = ui->addressText->fontMetrics().boundingRect(ui->addressText->text()).width();
    }

    int privateKeyTextLength = ui->privateKeyText->fontMetrics().boundingRect(ui->privateKeyText->text()).width();
    font = ui->privateKeyText->font();
    for(int i = 0; i < PAPER_WALLET_READJUST_LIMIT; i++) {
        if ( privateKeyTextLength < minTextWidth) {
            font.setPixelSize(font.pixelSize() + pixelSizeStep);
            ui->privateKeyText->setFont(font);
            privateKeyTextLength = ui->privateKeyText->fontMetrics().boundingRect(ui->privateKeyText->text()).width();
        } else {
            break;
        }
    }
    if ( privateKeyTextLength > maxTextWidth ) {
        font.setPixelSize(font.pixelSize() - pixelSizeStep);
        ui->privateKeyText->setFont(font);
        privateKeyTextLength = ui->privateKeyText->fontMetrics().boundingRect(ui->privateKeyText->text()).width();
    }

}

void PaperWalletDialog::on_printButton_clicked()
{

    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *qpd = new QPrintDialog(&printer, this);

    qpd->setPrintRange(QAbstractPrintDialog::AllPages);

    QList<QString> recipientPubKeyHashes;

    if ( qpd->exec() != QDialog::Accepted ) {
        return;
    }

    // Hardcode these values
    printer.setOrientation(QPrinter::Portrait);
    printer.setPaperSize(QPrinter::A4);
    printer.setFullPage(true);

    QPainter painter;
    if (! painter.begin(&printer)) { // failed to open file
        QMessageBox::critical(this, "Printing Error", tr("failed to open file, is it writable?"), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    int walletCount = ui->walletCount->currentIndex() + 1;
    int walletsPerPage = 4;

    int pageHeight = printer.pageRect().height() - PAPER_WALLET_PAGE_MARGIN;
    int walletHeight = ui->paperTemplate->height();
    double computedWalletHeight = 0.9 * pageHeight / walletsPerPage;
    double scale = computedWalletHeight / walletHeight;
    double walletPadding = pageHeight * 0.05 / (walletsPerPage - 1) / scale;

    QRegion walletRegion = QRegion(ui->paperTemplate->x(), ui->paperTemplate->y(),
    ui->paperTemplate->width(), ui->paperTemplate->height());
        painter.scale(scale, scale);

    for(int i = 0; i < walletCount; i++) {

        QPoint point = QPoint(PAPER_WALLET_PAGE_MARGIN, (PAPER_WALLET_PAGE_MARGIN / 2) + ( i % walletsPerPage ) * (walletHeight + walletPadding));
        this->render(&painter, point, walletRegion);
        recipientPubKeyHashes.append(ui->addressText->text());

        if ( i % walletsPerPage == ( walletsPerPage - 1 ) ) {

            printer.newPage();

        }

        this->on_getNewAddress_clicked();

    }

    painter.end();

#ifdef ENABLE_WALLET
    QStringList formatted;

    WalletModelTransaction *tx;
    while( true ) {
        bool ok;

        // Ask for an amount to send to each paper wallet. It might be better to try to use the BitcoinAmountField, but this works fine.
        double amountInput = QInputDialog::getDouble(this, tr("Load Paper Wallets"), tr("The paper wallet printing process has begun.<br/>Please wait for the wallets to print completely and verify that everything printed correctly.<br/>Check for misalignments, ink bleeding, smears, or anything else that could make the private keys unreadable.<br/>Now, enter the number of XHC you wish to send to each wallet:"), 0, 0, 2147483647, 8, &ok);

        if(!ok) {
            return;
        }


        WalletModel::UnlockContext ctx(this->model->requestUnlock());
        if(!ctx.isValid())
        {
            return;
        }

        QList<SendCoinsRecipient> recipients;
        quint64 amount = (quint64) ( amountInput * COIN );
        foreach(const QString &dest, recipientPubKeyHashes)
        {

            recipients.append(SendCoinsRecipient(dest,tr("Paper wallet %1").arg(dest), amount,""));
            formatted.append(tr("<b>%1</b> to Paper Wallet <span style='font-family: monospace;'>%2</span>").arg(QString::number(amountInput, 'f', 8), GUIUtil::HtmlEscape(dest)));

        }

        tx = new WalletModelTransaction(recipients);

        WalletModel::SendCoinsReturn prepareStatus;
        if (this->model->getOptionsModel()->getCoinControlFeatures()) // coin control enabled
            prepareStatus = this->model->prepareTransaction(*tx, CoinControlDialog::coinControl);
        else
            prepareStatus = this->model->prepareTransaction(*tx);

        if (prepareStatus.status == WalletModel::InvalidAddress) {
            QMessageBox::critical(this, tr("Send Coins"), tr("The recipient address is not valid, please recheck."), QMessageBox::Ok, QMessageBox::Ok);
        } else if (prepareStatus.status == WalletModel::InvalidAmount) {
            QMessageBox::critical(this, tr("Send Coins"), tr("The amount to pay must be larger than 0"), QMessageBox::Ok, QMessageBox::Ok);
        } else if (prepareStatus.status == WalletModel::AmountExceedsBalance) {
            QMessageBox::critical(this, tr("Send Coins"), tr("The amount exceeds your balance."), QMessageBox::Ok, QMessageBox::Ok);
        } else if (prepareStatus.status == WalletModel::AmountWithFeeExceedsBalance) {
            QMessageBox::critical(this, tr("Send Coins"), tr("The total exceeds your balance when the transaction fee is included"), QMessageBox::Ok, QMessageBox::Ok);
        } else if (prepareStatus.status == WalletModel::DuplicateAddress) {
            QMessageBox::critical(this, tr("Send Coins"), tr("Duplicate address found, can only send to each address once per send operation."), QMessageBox::Ok, QMessageBox::Ok);
        } else if (prepareStatus.status == WalletModel::TransactionCreationFailed) {
            QMessageBox::critical(this, tr("Send Coins"), tr("Transaction creation failed!"), QMessageBox::Ok, QMessageBox::Ok);
        } else if (prepareStatus.status == WalletModel::OK) {
            break;
        } else {
            delete tx;
            return;
        }

    }

   // Stolen from sendcoinsdialog.cpp
    qint64 txFee = tx->getTransactionFee();
    QString questionString = tr("Are you sure you want to send?");
    questionString.append("<br /><br />%1");

    if(txFee > 0)
    {
        // append fee string if a fee is required
        questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), txFee));
        questionString.append("</span> ");
        questionString.append(tr("added as transaction fee"));
    }

    // add total amount in all subdivision units
    questionString.append("<hr />");
    qint64 totalAmount = tx->getTotalTransactionAmount() + txFee;
    QStringList alternativeUnits;
    foreach(BitcoinUnits::Unit u, BitcoinUnits::availableUnits())
    {
        if(u != model->getOptionsModel()->getDisplayUnit())
            alternativeUnits.append(BitcoinUnits::formatWithUnit(u, totalAmount));
    }

    questionString.append(tr("Total Amount %1 (= %2)")
        .arg(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), totalAmount))
        .arg(alternativeUnits.join(" " + tr("or") + " ")));

    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm send coins"),
        questionString.arg(formatted.join("<br />")),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        delete tx;
        return;
    }

    WalletModel::SendCoinsReturn sendStatus = this->model->sendCoins(*tx);

    if (sendStatus.status == WalletModel::TransactionCommitFailed) {
        QMessageBox::critical(this, tr("Send Coins"), tr("The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here."), QMessageBox::Ok, QMessageBox::Ok);
    }
    delete tx;
#endif
    return;

}


/** "Help message" dialog box */
HelpMessageDialog::HelpMessageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpMessageDialog)
{
    ui->setupUi(this);
    GUIUtil::restoreWindowGeometry("nHelpMessageDialogWindow", this->size(), this);

    header = tr("Hostcoin Core") + " " + tr("version") + " " +
        QString::fromStdString(FormatFullVersion()) + "\n\n" +
        tr("Usage:") + "\n" +
        "  hostcoin-qt [" + tr("command-line options") + "]                     " + "\n";

    coreOptions = QString::fromStdString(HelpMessage(HMM_BITCOIN_QT));

    uiOptions = tr("UI options") + ":\n" +
        "  -choosedatadir            " + tr("Choose data directory on startup (default: 0)") + "\n" +
        "  -lang=<lang>              " + tr("Set language, for example \"de_DE\" (default: system locale)") + "\n" +
        "  -min                      " + tr("Start minimized") + "\n" +
        "  -rootcertificates=<file>  " + tr("Set SSL root certificates for payment request (default: -system-)") + "\n" +
        "  -splash                   " + tr("Show splash screen on startup (default: 1)");

    ui->helpMessageLabel->setFont(GUIUtil::bitcoinAddressFont());

    // Set help message text
    ui->helpMessageLabel->setText(header + "\n" + coreOptions + "\n" + uiOptions);
}

HelpMessageDialog::~HelpMessageDialog()
{
    GUIUtil::saveWindowGeometry("nHelpMessageDialogWindow", this);
    delete ui;
}

void HelpMessageDialog::printToConsole()
{
    // On other operating systems, the expected action is to print the message to the console.
    QString strUsage = header + "\n" + coreOptions + "\n" + uiOptions + "\n";
    fprintf(stdout, "%s", strUsage.toStdString().c_str());
}

void HelpMessageDialog::showOrPrint()
{
#if defined(WIN32)
        // On Windows, show a message box, as there is no stderr/stdout in windowed applications
        exec();
#else
        // On other operating systems, print help text to console
        printToConsole();
#endif
}

void HelpMessageDialog::on_okButton_accepted()
{
    close();
}


/** "Shutdown" window */
void ShutdownWindow::showShutdownWindow(BitcoinGUI *window)
{
    if (!window)
        return;

    // Show a simple window indicating shutdown status
    QWidget *shutdownWindow = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QLabel(
        tr("Hostcoin Core is shutting down...") + "<br /><br />" +
        tr("Do not shut down the computer until this window disappears.")));
    shutdownWindow->setLayout(layout);

    // Center shutdown window at where main window was
    const QPoint global = window->mapToGlobal(window->rect().center());
    shutdownWindow->move(global.x() - shutdownWindow->width() / 2, global.y() - shutdownWindow->height() / 2);
    shutdownWindow->show();
}
