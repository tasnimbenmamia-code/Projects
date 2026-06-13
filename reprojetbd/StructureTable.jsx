import React, { useState, useEffect } from 'react';
import { DataTable } from 'primereact/datatable';
import { Column } from 'primereact/column';
import { Toolbar } from 'primereact/toolbar';
import { Button } from 'primereact/button';
import StructureDialog from './StructureDialog';
import ConfirmDialogBox from './ConfirmDialogBox';
import { getStructures,deleteStructure } from '../services/StructureService';

const StructureTable = () => {
    const [Structures, setStructures] = useState([]);
    const [selectedStructure, setSelectedStructure] = useState(null);
    const [selectedStructures, setSelectedStructures] = useState([]);
    const [dialogVisible, setDialogVisible] = useState(false);
    const [confirmVisible, setConfirmVisible] = useState(false);

    useEffect(() => {
        fetchStructures();
    }, []);

    const fetchStructures = () => {
        getStructures().then(response => {
            setStructures(response.data);
        }).catch(error => console.error(error));
    };

    const openNew = () => {
        setSelectedStructure(null);
        setDialogVisible(true);
    };

    const editStructure = (structure) => {
        setSelectedStructure(structure);
        setDialogVisible(true);
    };

    const deleteStructureById = (structure) => {
        deleteStructure(structure.id)
            .then(() => {
                fetchStructures();
                setConfirmVisible(false);
            })
            .catch(error => console.error(error));
    };

    const leftToolbarTemplate = () => (
        <>
            <Button label="Nouveau" icon="pi pi-plus" className="p-button-success" onClick={openNew} />
            <Button label="Supprimer" icon="pi pi-trash" className="p-button-danger"
                disabled={!selectedStructures || !selectedStructures.length}
                onClick={() => setConfirmVisible(true)} />
        </>
    );

    const actionBodyTemplate = (rowData) => (
        <>
<Button
  style={{ width: '50px', alignItems: 'center' , marginLeft:'8%'}}
  icon="pi pi-pencil"
  className="p-button-rounded p-button-success mr-2"
  onClick={() => editStructure(rowData)}
/>            <Button style={{width:'50px'}}  icon="pi pi-trash" className="p-button-rounded p-button-warning" onClick={() => deleteStructureById(rowData)} />
        </>
    );

    return (
        <div className="card">
            <Toolbar className="mb-4" left={leftToolbarTemplate} />
            <DataTable value={Structures} selection={selectedStructures} onSelectionChange={e => setSelectedStructures(e.value)}
                dataKey="id" paginator rows={10} header="Liste des Structures">
                <Column selectionMode="multiple" headerStyle={{ width: '3em' }}></Column>
                <Column field="id" header="Id" sortable></Column>
                <Column field="libelle" header="Libelle" sortable></Column>
                <Column body={actionBodyTemplate}></Column>
            </DataTable>

            {/* Dialogue pour ajout/édition */}
            <StructureDialog visible={dialogVisible} 
                               onHide={() => setDialogVisible(false)} 
                               structure={selectedStructure} 
                               refresh={fetchStructures} />

            {/* Dialogue de confirmation (delete multiple ou confirmation simple) */}
            <ConfirmDialogBox visible={confirmVisible} 
                               onHide={() => setConfirmVisible(false)}
                               onConfirm={() => {
                                   selectedStructures.forEach(p => deleteStructureById(p));
                                   setSelectedStructures([]);
                               }} 
                               message="Confirmez-vous la suppression des structures sélectionnés ?" />
        </div>
    );
};

export default StructureTable;