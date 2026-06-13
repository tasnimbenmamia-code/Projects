import React, { useState, useEffect } from 'react';
import { Dialog } from 'primereact/dialog';
import { InputText } from 'primereact/inputtext';
import { Button } from 'primereact/button';
import { createUser, updateUser } from '../services/UserService';
import { getRoles } from '../services/RoleService';
import { Dropdown } from 'primereact/dropdown';

const RoleDialog = ({ visible, onHide, user, refresh }) => {
    const [formData, setFormData] = useState({
        id: '',
        login: '',
        password: '',
        role_ID: '',
        role: ''
    });

    useEffect(() => {
        if (user) {
            setFormData({
                id: user.id,
                login: user.login,
                password: user.password,
                role_ID: user.idRole.id,
                role: user.idRole.nom
            });
        } else {
            setFormData({
                id: '',
                login: '',
                password: '',
                role_ID: '',
                role: ''
            });
        }
    }, [user]);

    const [roles, setRoles] = useState([]);

    useEffect(() => {
        getRoles().then(res => setRoles(res.data));
    }, []);
  

    const onSubmit = () => {
        console.log("sending User:", formData);
        const userToSend = {
            login: formData.login,
            password: formData.password,
           
            idRole: {
                id: parseInt(formData.role_ID)
            }
        };
        if (user && user.id) {
            updateUser(user.id, userToSend)  
            .then(() => {
                refresh();
                onHide();
            })} 
            else {
            createUser(userToSend)
            .then(() => {
                refresh();
                onHide();
            })
            .catch(error => console.error(error));
        }
    };

    const footer = (
        <>
            <Button label="Annuler" icon="pi pi-times" onClick={onHide} className="p-button-secondary" />
            <Button label="Enregistrer" icon="pi pi-check" onClick={onSubmit} autoFocus />
        </>
    );

    return (
        <Dialog header="Détails User" visible={visible} style={{ width: '30vw' }} footer={footer} onHide={onHide}>

            <div className="p-field">
                <label htmlFor="login">Login</label>
                <InputText id="login" value={formData.login} onChange={(e) => setFormData({ ...formData, login: e.target.value })} />
            </div>
            <div className="p-field">
                <label htmlFor="password">Password</label>
                <InputText id="password" value={formData.password} onChange={(e) => setFormData({ ...formData, password: e.target.value })} />
            </div>
          
            <div className="p-field">
                <label htmlFor="id">Role</label>
                <Dropdown
                 id="role"
                 value={formData.role_ID}
                 options={roles}
                 onChange={(e) => setFormData({ ...formData, role_ID: e.value })}
                 optionLabel="nom"
                 optionValue="id"
                 placeholder="Choisir un role"
                />
            </div>
            <div className="p-field">
                <label htmlFor="roleid">Role_ID</label>
                <InputText id="roleid" value={formData.role_ID} onChange={(e) => setFormData({ ...formData, role_ID: e.target.value })} />
            </div>
        </Dialog>
    );
};

export default RoleDialog;