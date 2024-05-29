package com.example.javaapi.db;

import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;

@Entity
public class Table1 {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer uuid;

    private String columnA;
    private String columnB;
    private String columnC;
    private String columnD;

    // Getters and setters

    public Integer getUuid() {
        return uuid;
    }

    public void setUuid(Integer uuid) {
        this.uuid = uuid;
    }

    public String getColumnA() {
        return columnA;
    }

    public void setColumnA(String columnA) {
        this.columnA = columnA;
    }

    public String getColumnB() {
        return columnB;
    }

    public void setColumnB(String columnB) {
        this.columnB = columnB;
    }

    public String getColumnC() {
        return columnC;
    }

    public void setColumnC(String columnC) {
        this.columnC = columnC;
    }

    public String getColumnD() {
        return columnD;
    }

    public void setColumnD(String columnD) {
        this.columnD = columnD;
    }
}
