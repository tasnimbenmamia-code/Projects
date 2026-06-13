import React, { useState, useEffect } from 'react';
import { DataTable } from 'primereact/datatable';
import { Column } from 'primereact/column';
import { Toolbar } from 'primereact/toolbar';
import { Button } from 'primereact/button';
import ParticipantDialog from './ParticipantDialog';
import ConfirmDialogBox from './ConfirmDialogBox';
import { getParticipants,deleteParticipant } from '../services/ParticipantService';

const ParticipantTable = () => {
    const [Participants, setParticipants] = useState([]);
    const [selectedParticipant, setSelectedParticipant] = useState(null);
    const [selectedParticipants, setSelectedParticipants] = useState([]);
    const [dialogVisible, setDialogVisible] = useState(false);
    const [confirmVisible, setConfirmVisible] = useState(false);

    useEffect(() => {
        fetchParticipants();
    }, []);

    const fetchParticipants = () => {
        getParticipants().then(response => {
            setParticipants(response.data);
        }).catch(error => console.error(error));
    };

    const openNew = () => {
        setSelectedParticipant(null);
        setDialogVisible(true);
    };

    const editParticipant = (participant) => {
        setSelectedParticipant(participant);
        setDialogVisible(true);
    };

    const deleteParticipantById = (participant) => {
        deleteParticipant(participant.id)
            .then(() => {
                fetchParticipants();
                setConfirmVisible(false);
            })
            .catch(error => console.error(error));
    };

    const leftToolbarTemplate = () => (
        <>
            <Button label="Nouveau" icon="pi pi-plus" className="p-button-success" onClick={openNew} />
            <Button label="Supprimer" icon="pi pi-trash" className="p-button-danger"
                disabled={!selectedParticipants || !selectedParticipants.length}
                onClick={() => setConfirmVisible(true)} />
        </>
    );

    const actionBodyTemplate = (rowData) => (
        <>
<Button
  style={{ width: '50px', alignItems: 'center' , marginLeft:'8%'}}
  icon="pi pi-pencil"
  className="p-button-rounded p-button-success mr-2"
  onClick={() => editParticipant(rowData)}
/>            <Button style={{width:'50px'}}  icon="pi pi-trash" className="p-button-rounded p-button-warning" onClick={() => deleteParticipantById(rowData)} />
        </>
    );

    return (
        <div className="card">
            <Toolbar className="mb-4" left={leftToolbarTemplate} />
            <DataTable value={Participants} selection={selectedParticipants} onSelectionChange={e => setSelectedParticipants(e.value)}
                dataKey="id" paginator rows={10} header="Liste des Fonctions">
                <Column selectionMode="multiple" headerStyle={{ width: '3em' }}></Column>
                <Column field="id" header="Id" sortable></Column>
                <Column field="email" header="email" sortable></Column>
                <Column field="nom" header="nom" sortable></Column>
                <Column field="prenom" header="prenom" sortable></Column>
                <Column field="tel" header="tel" sortable></Column>
                <Column field="idprofil.id" header="profil_ID" sortable></Column>
                <Column field="idprofil.nom" header="profil" sortable></Column>
                <Column field="idstructure.id" header="structure_ID" sortable></Column>
                <Column field="idstructure.nom" header="structure" sortable></Column>
                <Column body={actionBodyTemplate}></Column>
            </DataTable>

            {/* Dialogue pour ajout/édition */}
            <ParticipantDialog visible={dialogVisible} 
                               onHide={() => setDialogVisible(false)} 
                               participant={selectedParticipant} 
                               refresh={fetchParticipants} />

            {/* Dialogue de confirmation (delete multiple ou confirmation simple) */}
            <ConfirmDialogBox visible={confirmVisible} 
                               onHide={() => setConfirmVisible(false)}
                               onConfirm={() => {
                                   selectedParticipants.forEach(p => deleteParticipantById(p));
                                   setSelectedParticipants([]);
                               }} 
                               message="Confirmez-vous la suppression des participant sélectionnés ?" />
        </div>
    );
};

export default ParticipantTable;